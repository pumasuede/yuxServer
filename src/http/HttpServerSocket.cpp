#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <fstream>

#include "HttpServerSocket.h"
#include "HttpConfig.h"
#include "base/Log.h"
#include "base/Config.h"
#include "base/Server.h"

using namespace std;
using namespace yux::base;
using namespace yux::parser;

namespace yux{
namespace http{

std::mutex HttpServerThread::mutex_;
std::condition_variable HttpServerThread::cv_;
std::mutex HttpServerThread::socketMutex_;

HttpServerSocket* HttpServerSocket::create(const string& host, uint16_t port)
{
    HttpServerSocket* pSock = new HttpServerSocket(host, port);
    pSock->setCbRead(std::bind(&HttpServerSocket::readCallBack, pSock, _1, _2, _3));

    string docRoot = Config::getInstance()->get("document_root", ".");
    string mimeFile = Config::getInstance()->get("mime_file", "mime_types");
    pSock->setDocRoot(docRoot);
    pSock->setMimeFile(mimeFile);

    MimeConfig* pMiMeConfig = MimeConfig::getInstance();
    pMiMeConfig->loadConfigFile(mimeFile);
    return pSock;
}

void HttpServerSocket::setDocRoot(const string& docRoot)
{
    docRoot_ = docRoot;
}

void HttpServerSocket::setMimeFile(const string& mimeFile)
{
    mimeFile_ = mimeFile;
}

int HttpServerSocket::readCallBack(const char* buf, size_t size, SocketBase *sock)
{
    static int seq = 1;
    static string recvData;
    string tmpData(buf, size);
    recvData += tmpData;
    log_trace("  <RAW tmp data>:%s...\n", tmpData.substr(0, 30).c_str());
    if (tmpData.find("\r\n\r\n") == string::npos)
    {
        log_debug("Wait for more data");
        return 0;
    }

    Req req;
    req.fd = sock->fd();
    req.sock = sock;
    req.data = recvData;
    log_trace("\n<Recv HTTP seq:%d fd:%d>:\n%s", seq, req.fd, recvData.c_str());

    if (recvData.size() > 0)
    {
        lock_guard<std::mutex> lock(HttpServerThread::mutex_);
        req.seq = seq++;
        reqList_.push_back(req);
        HttpServerThread::cv_.notify_all();
    }

    recvData.clear();
}

void HttpServerThread::handleScript(HttpRequest& httpReq)
{
    const string& scriptName = httpReq.startLine.scriptName;
    const string& queryString = httpReq.startLine.queryString;

    string fileExt = scriptName.substr(scriptName.find_last_of('.') + 1);
    string localFile = pServerSock_->getDocRoot() + scriptName;

    // call fasct CGI to process php script
    log_debug("Preparing to run script - %s", scriptName.c_str());
    fastCgi_.setPort(fileExt == "php" ? 9000: 9100);
    fastCgi_.setRequestId(req_.seq);

    fastCgi_.setParams("SCRIPT_FILENAME", localFile);
    fastCgi_.setParams("QUERY_STRING", queryString);
    fastCgi_.setParams("REQUEST_METHOD", httpReq.startLine.method);
    fastCgi_.setParams("CONTENT_TYPE", httpReq.header["Content-Type"]);
    fastCgi_.setParams("CONTENT_LENGTH", httpReq.header["Content-Length"]);
    fastCgi_.setParams("SCRIPT_NAME", scriptName);
    fastCgi_.setParams("DOCUMENT_URI", httpReq.startLine.URI);
    fastCgi_.setParams("DOCUMENT_ROOT", pServerSock_->getDocRoot());
    fastCgi_.setParams("SERVER_PROTOCOL", "HTTP/1.1");
    fastCgi_.setParams("GATEWAY_INTERFACE", "CGI/1.1");
    fastCgi_.setParams("SERVER_SOFTWARE", "yuHttpd/1.0");
    fastCgi_.setParams("HTTP_HOST", httpReq.header["Host"]);
    fastCgi_.setParams("HTTP_USER_AGENT", httpReq.header["User-Agent"]);
    fastCgi_.setParams("HTTP_ACCEPT", httpReq.header["Accept"]);

    if (httpReq.startLine.method == "POST" || httpReq.startLine.method == "PUT")
    {
        log_trace("send %s data: %s", httpReq.startLine.method.c_str(), httpReq.body.c_str());
        fastCgi_.setPostData(httpReq.body);
    }

    fastCgi_.sendRequest();
    fastCgi_.readFromCGI(req_.sock);
    fastCgi_.endRequest();
}

void HttpServerThread::workBody()
{
    list<Req>& reqList = pServerSock_->getReqList();

    while (1)
    {
        {
            unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [&]{return reqList.size() > 0;});

            req_ = reqList.front();
            reqList.pop_front();
            log_debug("Fetch one request from queue");
        }

        HttpRequest httpReq;

        bool parseRet = httpParser_.parse(httpReq, req_.data.c_str(), req_.data.size());
        const string& uri = httpReq.startLine.URI;
        log_debug("Process http request seq:%d uri:%s", req_.seq, uri.c_str());

        if (!parseRet)
        {
            log_error("parse http error");
            lock_guard<std::mutex> lock(HttpServerThread::socketMutex_);
            Server::getInstance().closeSocket(req_.fd);
            continue;
        }

        SocketBase *sock;

        // Check if the socket is still valid in case it's closed by remote peer.
        {
            lock_guard<std::mutex> lock(HttpServerThread::socketMutex_);
            sock = Server::getInstance().getSocketByFd(req_.fd);
        }

        if (sock == nullptr || sock != req_.sock)
        {
            log_error("The socket on fd %d is already closed, nothing to do", req_.fd);
            continue;
        }


        const string& scriptName = httpReq.startLine.scriptName;
        string localFile = pServerSock_->getDocRoot() + scriptName;

        string fileExt = scriptName.substr(scriptName.find_last_of('.') + 1);
        string contentType = MimeConfig::getInstance()->get(fileExt, "");
        bool isBin = isBinary(contentType);
        ifstream file(localFile.c_str(), isBin ? ios::binary : ios::in);

        if (file.is_open())
        {
            sock->sendStr("HTTP/1.1 200 OK\r\n");
            sock->sendStr("Server: yuhttpd\r\n");
            // process php script
            if (fileExt == "php" || fileExt == "cgi")
            {
                handleScript(httpReq);
            }
            else
            {
                // process static request
                int fileSize = getFileSize(localFile);
                log_debug("Process static %s file request. File size: %d", isBin ? "binary" : "text", fileSize);

                sock->sendStr("Content-Type: " + contentType + "\r\n");

                if (isBin)
                {
                    sock->sendStr("Content-Length: " + to_string(fileSize) + "\r\n");
                }
                sock->sendStr("Connection: Keep-Alive\r\n");

                sock->sendStr("\r\n");

                std::array<char, 4096> buf;
                int bufSize = buf.size();

                while (file.good())
                {
                    size_t readLen = file.read(buf.data(), bufSize).gcount();
                    sock->write((uint8_t*)buf.data(), readLen);
                }
            }
            file.close();
        }
        else
        {
            // Todo return 404
            sock->sendStr("HTTP/1.1 404 OK\r\n");
            sock->sendStr("Server: yuhttpd\r\n");
            sock->sendStr("Can't find Request URL "+ uri +" !<br>\n");
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            Server::getInstance().closeSocket(req_.fd);
        }

        log_debug("Handle request done, exit workBody\n");
    }
}

}} // name space
