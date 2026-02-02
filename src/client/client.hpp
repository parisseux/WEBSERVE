#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>
# include <fcntl.h>
# include "../request/Request.hpp"

# define MAX_PENDING 20

enum ClientState
{
    WAITING,
    READING_HEADER,
    READING_BODY
};

class Client 
{
    private:
        int         _fd;
        int         _flags;
        Request     _request;
        std::string _requestBuffer;
        std::string _responseBuffer;
        bool        _ReadyToWrite;
        ClientState _state;            
    public:
        Client();
        ~Client();
        void         setFd(int fd);
        void         setFlags(int flags);
        void         setRequestBuffer(std::string requestBuffer);
        void         setResponseBuffer(std::string _responseBuffer);
        void         setReadyToWrite(bool ReadyToWrite);
        void         setClientState(ClientState state); 
        int&         getFd();
        int&         getFlags();
        std::string& getRequestBuffer();
        std::string& getResponseBuffer();
        bool&        getReadyToWrite();
        Request&     getRequestClass();
        ClientState  getClientState();
        void         clearRequest();
        int          getContentLength();
};

void setNonBlocking(int fd);

#endif
