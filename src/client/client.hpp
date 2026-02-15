#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>
# include <fcntl.h>
# include <deque>
# include "../request/Request.hpp"
# include "../response/Response.hpp"

# define MAX_PENDING 20

enum ClientState
{
    WAITING,
    READING_HEADER,
    READING_BODY,
    GENERATING_RESPONSE,
    GENERATING_CGI,
    SENDING_RESPONSE,
};

class Request;

class Client 
{
    private:
        int         _fd;
        int         _cgi_fd;
        int         _flags;
        Request     _request;
        Response    _response;
        std::string _requestBuffer;
        std::deque<std::string> _responseBuffer;
        bool        _bodyComplete;
        bool        _ReadyToWrite;
        int         _byteSentPos;
        ClientState _state;    
    public:
        Client();
        ~Client();
        void         setFd(int fd);
        void         setFlags(int flags);
        void         setRequestBuffer(std::string requestBuffer);
        void         setResponseBuffer(std::deque<std::string> _responseBuffer);
        void         setReadyToWrite(bool ReadyToWrite);
        void         setClientState(ClientState state); 
        int&         getFd();
        int&         getFlags();
        std::string& getRequestBuffer();
        std::deque<std::string>& getResponseBuffer();
        bool&        getReadyToWrite();
        Request&     getRequestClass();
        Response&    getResponseClass(){return _response;}
        ClientState  getClientState();
        void         clearRequest();
        void         clearResponse();
        int          getContentLength();
        int          getResponseBufferLength();
        void         setCgiFd(int fd){_cgi_fd = fd;}
        int          getCgiFd(){return(_cgi_fd);}
        void         addByteSentPos(int byte){_byteSentPos += byte;}
        int          getByteSentPos(){return _byteSentPos;} 
        void         setByteSentPos(int byte){_byteSentPos = byte;}       
        void         setBodyComplete(bool bodyComplete){_bodyComplete = bodyComplete;}
        bool         getBodyComplete(){return (_bodyComplete);}
        void         Handle(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server, Client *client, Epoll &epoll);
        void         clearClient();      
};

void setNonBlocking(int fd);

#endif
