#include "fast_cgi.h"
#include "fcgi.h"
#include "base/Log.h"
#include "common/Utils.h"

#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

using namespace std;
using namespace yux::base;
using namespace yux::common;

namespace yux{
namespace http{

FastCgi::FastCgi(const string& ip, uint16_t port) : ip_(ip), port_(port)
{
    sockFd_ = 0;
    requestId_ = 0;
}

FastCgi::~FastCgi()
{
    ::close(sockFd_);
}

FCGI_Header FastCgi::makeHeader(int type, int requestId, int contentLength, int paddingLength)
{
    FCGI_Header header;

    header.version = FCGI_VERSION_1;
    header.type    = (unsigned char)type;
    writeNum(&header.requestIdB1, requestId, 2);
    writeNum(&header.contentLengthB1, contentLength, 2);
    header.paddingLength = (unsigned char)paddingLength;
    header.reserved = 0;

    return header;
}

FCGI_BeginRequestBody FastCgi::makeBeginRequestBody(int role, int keepConnection)
{
    FCGI_BeginRequestBody body;

    writeNum(&body.roleB1, role, 2);
    body.flags = (unsigned char)((keepConnection) ? FCGI_KEEP_CONN : 0);

    bzero(&body.reserved, sizeof(body.reserved));

    return body;
}

FCGI_EndRequestBody FastCgi::makeEndRequestBody(int appStatus, unsigned char protocolStatus)
{
    FCGI_EndRequestBody body;

    writeNum(&body.appStatusB3, appStatus, 4);
    body.protocolStatus= protocolStatus;

    return body;
}

bool FastCgi::makeNameValueBody(std::string name, std::string value,
                                unsigned char *bodyBuffPtr, int *bodyLenPtr)
{
    unsigned char *startBodyBuffPtr = bodyBuffPtr;

    int nameLen = name.size();
    int valueLen = value.size();

    if (nameLen < 128)
    {
        // 1 byte to store length
        *bodyBuffPtr++ = (unsigned char)nameLen;
    }
    else
    {
        // 4 bytes to store length
        writeNum(bodyBuffPtr, nameLen, 4);
        bodyBuffPtr += 4;
    }

    if (valueLen < 128)
    {
        // 1 byte to store length
        *bodyBuffPtr++ = (unsigned char)valueLen;
    }
    else
    {
        // 4 bytes to store length
        writeNum(bodyBuffPtr, valueLen, 4);
        bodyBuffPtr += 4;
    }

    memcpy(bodyBuffPtr, name.c_str(), nameLen);
    bodyBuffPtr += nameLen;

    memcpy(bodyBuffPtr, value.c_str(), valueLen);
    bodyBuffPtr += valueLen;

    *bodyLenPtr = bodyBuffPtr - startBodyBuffPtr;
    return true;
}

void FastCgi::startConnect()
{
    struct sockaddr_in server_address;
    bzero(&server_address,sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_.c_str());
    server_address.sin_port = htons(port_);

    sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
    int result = connect(sockFd_, (struct sockaddr*)&server_address, sizeof(server_address));
    if (result == -1)
    {
        log_error("connect fast CGI manager failed: %s", strerror(errno));
    }
}

void FastCgi::close()
{
    ::close(sockFd_);
    sockFd_ = 0;
}

void FastCgi::setStartRequestRecord()
{
    reqBuff_.clear();

    FCGI_BeginRequestRecord beginRecord;
    beginRecord.header = makeHeader(FCGI_BEGIN_REQUEST, requestId_, sizeof(beginRecord.body), 0);
    beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER, 0);

    reqBuff_ += string((char*)&beginRecord, sizeof(beginRecord));
}

void FastCgi::endRequest(void)
{
    paramBody_.clear();
    data_.clear();
    reqBuff_.clear();

    FCGI_EndRequestRecord endRecord;
    endRecord.header = makeHeader(FCGI_END_REQUEST, requestId_, 0, 0);
    endRecord.body = makeEndRequestBody(0, FCGI_REQUEST_COMPLETE);

    reqBuff_ += string((char*)&endRecord, sizeof(endRecord));
    write(sockFd_, reqBuff_.c_str(), reqBuff_.size());
    close();
}

void FastCgi::setParams(std::string name, std::string value)
{
    unsigned char paramBuff[PARAMS_BUFF_LEN] = { };
    int paramLen;

    makeNameValueBody(name, value, paramBuff, &paramLen);
    paramBody_ += string((char*)paramBuff, paramLen);
}

void FastCgi::setPostData(const std::string& data)
{
    data_ = data;
}

void FastCgi::sendRequest()
{
    log_debug("Send script to FastCGI manager. requestId_=%d", requestId_);
    // Start request
    setStartRequestRecord();

    // Add param
    FCGI_Header paramHeader = makeHeader(FCGI_PARAMS, requestId_, paramBody_.size(), 0);
    reqBuff_ += string((char*)&paramHeader, FCGI_HEADER_LEN);

    reqBuff_ += paramBody_;

    paramHeader = makeHeader(FCGI_PARAMS, requestId_, 0, 0);
    reqBuff_ += string((char*)&paramHeader, FCGI_HEADER_LEN);

    // Add data
    FCGI_Header dataHeader = makeHeader(FCGI_STDIN, requestId_, data_.size(), 0);
    reqBuff_ += string((char*)&dataHeader, FCGI_HEADER_LEN);

    reqBuff_ += data_;

    dataHeader = makeHeader(FCGI_STDIN, requestId_, 0, 0);
    reqBuff_ += string((char*)&dataHeader, FCGI_HEADER_LEN);

    startConnect();
    write(sockFd_, reqBuff_.c_str(), reqBuff_.size());
}

void FastCgi::readFromCGI(yux::base::SocketBase *pClientSock)
{
    FCGI_Header responderHeader;
    static uint8_t content[CONTENT_BUFF_LEN];
    static uint8_t tmp[256];
    int contentLen;
    int readBytes = 0;
    int ret;

    while (read(sockFd_, &responderHeader, FCGI_HEADER_LEN) > 0)
    {
        if (responderHeader.type == FCGI_STDOUT)
        {
            readBytes = 0;
            contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);
            if (!pClientSock)
                continue;

            while (readBytes < contentLen)
            {
                bzero(content, CONTENT_BUFF_LEN);

                int remain = contentLen - readBytes;
                int toRead = (remain < CONTENT_BUFF_LEN - 1) ? remain : CONTENT_BUFF_LEN - 1;
                ret = read(sockFd_, content, toRead);
                if (ret)
                {
                    readBytes += ret;
                    log_trace("processed CGI content:\n%s", content);
                    pClientSock->send(content, ret);
                }
                else if (ret < 0)
                {
                    log_error("Error: %s", strerror(errno));
                    break;
                }
                else
                {
                    break;
                }
            }

            if (responderHeader.paddingLength > 0)
            {
                read(sockFd_, tmp, responderHeader.paddingLength);
            }
        }
        else if (responderHeader.type == FCGI_STDERR)
        {
            readBytes = 0;
            contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);

            while (readBytes < contentLen)
            {
                bzero(content, CONTENT_BUFF_LEN);
                int remain = contentLen - readBytes;
                int toRead = (remain < CONTENT_BUFF_LEN - 1) ? remain : CONTENT_BUFF_LEN - 1;
                ret = read(sockFd_, content, toRead);
                if (ret)
                {
                    readBytes += ret;
                    log_error("Process CGI error: %s", content);
                    pClientSock->send(content, ret);
                }
                else if (ret < 0)
                {
                    log_error("Process CGI error: %s", strerror(errno));
                    break;
                }
                else
                {
                    break;
                }
            }

            if (responderHeader.paddingLength > 0)
            {
                read(sockFd_, tmp, responderHeader.paddingLength);
            }
        }
        else if (responderHeader.type == FCGI_END_REQUEST)
        {
            FCGI_EndRequestBody endRequest;
            read(sockFd_, &endRequest, sizeof(endRequest));
        }
    }
}

}} //name space;
