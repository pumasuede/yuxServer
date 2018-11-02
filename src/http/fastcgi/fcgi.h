#ifndef YUX_FCGI_H_
#define YUX_FCGI_H_

#include "fast_cgi.h"
#include "base/Socket.h"

#include <string>

namespace yux{
namespace http{

class FastCgi
{
public:
    FastCgi(const std::string& ip = "127.0.0.1", uint16_t port = 9000);
    ~FastCgi();

    void setIP(const std::string& ip) { ip_ = ip; }
    void setPort(uint16_t port) { port_ = port; }

    void setRequestId(int requestId) { requestId_ = requestId; }
    FCGI_Header makeHeader(int type,int request,
            int contentLength,int paddingLength);
    FCGI_BeginRequestBody makeBeginRequestBody(int role, int keepConnection);
    FCGI_EndRequestBody makeEndRequestBody(int appStatus, unsigned char protocolStatus);

    bool makeNameValueBody(std::string name, std::string value,
                           unsigned char *bodyBuffPtr, int *bodyLen);

    void startConnect();
    void close();
    void setStartRequestRecord();
    void setParams(std::string name, std::string value);
    void setPostData(const std::string& data);
    void sendRequest();
    void endRequest();
    void readFromCGI(yux::base::SocketBase *pClientSock = nullptr);

private:
    std::string ip_;   // IP of fastcgi process manager
    uint16_t port_;    // Port of fastcgi process manager
    int sockFd_;       // sockfd
    int requestId_;    // record ID
    std::string reqBuff_;
    int paramBodyLen_;
    std::string paramBody_;
    std::string data_;

    static const int PARAMS_BUFF_LEN = 1024;
    static const int CONTENT_BUFF_LEN = 1024;
};

}} // namespace
#endif
