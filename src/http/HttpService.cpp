#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <array>

#include "HttpService.h"
#include "HttpConfig.h"

#include "base/Log.h"
#include "common/Utils.h"
#include "common/Config.h"
#include "common/Server.h"

using namespace std;
using namespace yux::base;
using namespace yux::common;
using namespace yux::parser;

namespace yux{
namespace http{

HttpService* HttpService::create(const string& host, uint16_t port)
{
    Config *pConfig = Config::getInstance();
    string numWorkThread = pConfig->get("work_thread", "5");
    string docRoot = pConfig->get("document_root", ".");
    string mimeFile = Config::getInstance()->get("mime_file", "mime_types");

    MimeConfig* pMiMeConfig = MimeConfig::getInstance();
    pMiMeConfig->loadConfigFile(mimeFile);

    Scheduler *pScheduler = new Scheduler("HTTPService", stoi(numWorkThread));

    HttpService *pService = new HttpService(host, port, docRoot, pScheduler);
    return pService;
}

void HttpService::closeSocket(SocketBase* sock)
{
    if (recvInfoTable_.find(sock) != recvInfoTable_.end())
    {
        recvInfoTable_.erase(sock);
    }

    Server::getInstance()->closeSocket(sock);
}

void HttpService::onReadEvent(SocketBase *sock, const char* buf, size_t size)
{
    static uint32_t seq = 0;
    RecvInfo& recvInfo = recvInfoTable_[sock];
    string& recvBuf = recvInfo.recvBuf; // received data per socket.
    int& bodyCnt = recvInfo.bodyCnt;
    int& contentLength = recvInfo.contentLength;
    bool& hasBody = recvInfo.hasBody;

    string tmpData(buf, size);
    recvBuf += tmpData;
    log_trace("  <RAW tmp data>:%s...\n", tmpData.substr(0, 30).c_str());

    if (!hasBody && tmpData.find("\r\n\r\n") == string::npos)
    {
        log_debug("Wait for more data");
        return;
    }

    if (!hasBody)
    {
        // header is receieved. Parse it first to determin if we need to received more data;
        HttpRequest httpReq;
        bool parseRet = httpParser_.parse(httpReq, recvBuf.c_str(), recvBuf.size());
        if (!parseRet)
        {
            sock->sendStr("HTTP/1.1 411 Length Required\r\n");
            sock->sendStr("Server: yuHttpd/" HTTP_SERVER_VERSION "\r\n");

            log_fatal("parse http error");
            log_hexdump(recvBuf.c_str(), recvBuf.size());
            closeSocket(sock);
            return;
        }

        if (httpReq.startLine.method == "POST")
        {
            hasBody = true;
            bodyCnt = httpReq.body.size();
            contentLength = std::stoi(httpReq.header["Content-Length"]);
            if (contentLength == 0)
            {
                sock->sendStr("HTTP/1.1 411 Length Required\r\n");
                sock->sendStr("Server: yuHttpd/" HTTP_SERVER_VERSION "\r\n");

                log_fatal("Post request has no Content-Length");
                log_hexdump(recvBuf.c_str(), recvBuf.size());
                closeSocket(sock);
            }
        }
    }
    else
    {
        // Already received request header. continue to received body;
        bodyCnt += tmpData.size();
    }

    if (bodyCnt < contentLength)
    {
        // Wait for more data;
        return;
    }

    // Now we have the entire HTTP message.
    log_debug("<Recv HTTP seq:%d fd:%d  message size:%d, body size:%d",
              seq++, sock->fd(), recvBuf.size(), bodyCnt);
    log_trace_hexdump(recvBuf.c_str(), recvBuf.size());

    Req *pReq = new Req();
    pReq->fd = sock->fd();
    pReq->sock = sock;
    pReq->data = std::move(recvBuf);
    pReq->seq = seq;

    auto cb = bind(&HttpService::handleRequest, this, (void*) pReq);
    scheduler_->enqueueWorkItem(WorkItem(cb));

    recvInfoTable_.erase(sock);
}

void HttpService::handleStatic(SocketBase *sock, HttpRequest& httpReq, ifstream& fs)
{
    const string& scriptName = httpReq.startLine.scriptName;

    string fileExt = scriptName.substr(scriptName.find_last_of('.') + 1);
    string localFile = docRoot_ + scriptName;

    string contentType = MimeConfig::getInstance()->get(fileExt, "");
    bool isBin = isBinary(contentType);

    // process static request
    int fileSize = getFileSize(localFile);
    log_debug("Process static %s file request. File size: %d", isBin ? "binary" : "text", fileSize);

    sock->sendStr("Content-Type: " + contentType + "\r\n");
    sock->sendStr("Content-Length: " + to_string(fileSize) + "\r\n");
    sock->sendStr("Connection: Keep-Alive\r\n");
    sock->sendStr("\r\n");

    std::array<char, 8192> buf;
    int bufSize = buf.size();

    while (fs.good())
    {
        size_t readLen = fs.read(buf.data(), bufSize).gcount();
        int ret = sock->write((uint8_t*)buf.data(), readLen);
        if (ret == -1)
        {
            log_error("Error when writing socket buffer! errno=%d error str:%s ", errno, strerror(errno));
            break;
        }
    }
}

void HttpService::handleScript(SocketBase *sock, HttpRequest& httpReq)
{
    static uint32_t seq = 0;
    const string& scriptName = httpReq.startLine.scriptName;
    const string& queryString = httpReq.startLine.queryString;

    string fileExt = scriptName.substr(scriptName.find_last_of('.') + 1);
    string localFile = docRoot_ + scriptName;

    // call fasct CGI to process php script
    log_debug("Preparing to run script - %s", scriptName.c_str());
    fastCgi_.setPort(fileExt == "php" ? 9000: 9100);
    fastCgi_.setRequestId(++seq);

    fastCgi_.setParams("SCRIPT_FILENAME", localFile);
    fastCgi_.setParams("QUERY_STRING", queryString);
    fastCgi_.setParams("REQUEST_METHOD", httpReq.startLine.method);
    fastCgi_.setParams("CONTENT_TYPE", httpReq.header["Content-Type"]);
    fastCgi_.setParams("CONTENT_LENGTH", httpReq.header["Content-Length"]);
    fastCgi_.setParams("SCRIPT_NAME", scriptName);
    fastCgi_.setParams("DOCUMENT_URI", httpReq.startLine.URI);
    fastCgi_.setParams("DOCUMENT_ROOT", docRoot_);
    fastCgi_.setParams("SERVER_PROTOCOL", "HTTP/1.1");
    fastCgi_.setParams("GATEWAY_INTERFACE", "CGI/1.1");
    fastCgi_.setParams("SERVER_SOFTWARE", "yuHttpd/1.0");
    fastCgi_.setParams("HTTP_HOST", httpReq.header["Host"]);
    fastCgi_.setParams("HTTP_USER_AGENT", httpReq.header["User-Agent"]);
    fastCgi_.setParams("HTTP_ACCEPT", httpReq.header["Accept"]);

    if (httpReq.startLine.method == "POST" || httpReq.startLine.method == "PUT")
    {
        log_trace("send %s data. body size:%d", httpReq.startLine.method.c_str(), httpReq.body.size());
        fastCgi_.setPostData(httpReq.body);
    }

    fastCgi_.sendRequest();
    fastCgi_.readFromCGI(sock);
    fastCgi_.endRequest();
}

void HttpService::handleRequest(void *pArg)
{
    std::unique_ptr<Req> req((Req*)pArg);
    HttpRequest httpReq;

    // parse the entire http message.
    bool parseRet = httpParser_.parse(httpReq, req->data.c_str(), req->data.size());
    const string& uri = httpReq.startLine.URI;
    log_debug("Process http request seq:%d uri:%s", req->seq, uri.c_str());

    if (!parseRet)
    {
        log_fatal("parse http error");
        Server::getInstance()->closeSocket(req->sock);
        return;
    }

    shared_ptr<SocketBase> sock;

    // Check if the socket is still valid in case it's closed by remote peer.
    sock = Server::getInstance()->getSocketByFd(req->fd);

    if (!sock || sock.get() != req->sock || sock->fd() != req->fd)
    {
        log_error("The socket on fd %d is already closed, nothing to do", req->fd);
        return;
    }


    const string& scriptName = httpReq.startLine.scriptName;
    string localFile = docRoot_ + scriptName;

    string fileExt = scriptName.substr(scriptName.find_last_of('.') + 1);
    string contentType = MimeConfig::getInstance()->get(fileExt, "");
    bool isBin = isBinary(contentType);
    ifstream file(localFile.c_str(), isBin ? ios::binary : ios::in);

    if (file.is_open())
    {
        sock->sendStr("HTTP/1.1 200 OK\r\n");
        sock->sendStr("Server: yuHttpd/" HTTP_SERVER_VERSION "\r\n");
        // process php script
        if (fileExt == "php" || fileExt == "cgi")
        {
            handleScript(req->sock, httpReq);
        }
        else
        {
            handleStatic(req->sock, httpReq, file);
        }
        file.close();
    }
    else
    {
        string notFound = \
        "HTTP/1.1 404 Not Found\r\n"
        "Server: yuHttpd/" HTTP_SERVER_VERSION "\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n\r\n"
        "404 error! "+ uri +" not found.<br>\n";

        sock->sendStr(notFound);
    }

    closeSocket(sock.get());

    log_debug("Handle request done\n");
}

}} // name space
