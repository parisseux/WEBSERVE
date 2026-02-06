#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>
# include <fcntl.h>
# include <deque>
# include "../request/Request.hpp"

# define MAX_PENDING 20

enum ClientState
{
    WAITING,
    READING_HEADER,
    READING_BODY,
    GENERATING_BODY,
    SENDING_BODY
};

class Request;

class Client 
{
    private:
        int         _fd;
        int         _cgi_fd;
        int         _flags;
        Request     _request; 
        std::string _requestBuffer;
        std::deque<std::string> _responseBuffer;
        bool        _ReadyToWrite;
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
        ClientState  getClientState();
        void         clearRequest();
        int          getContentLength();
        int          getResponseBufferLength();
        void         setCgiFd(int fd){_cgi_fd = fd;}
        int          getCgiFd(){return(_cgi_fd);}
};

void setNonBlocking(int fd);

#endif
