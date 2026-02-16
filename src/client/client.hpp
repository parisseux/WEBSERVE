#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>
# include <vector>
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
class Epoll;

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
        Client() {std::cout << "Client constructor called" << std::endl;};
        ~Client() {std::cout << "Client destructor called" << std::endl;};

        int&         getFd() {return (this->_fd);};
        int&         getFlags() {return (this->_flags);};
        std::string& getRequestBuffer() {return (this->_requestBuffer);};
        std::deque<std::string>& getResponseBuffer(){return this->_responseBuffer;}        
        // std::string& getResponseBuffer() {return (this->_responseBuffer);};
        bool&        getReadyToWrite() {return (this->_ReadyToWrite);};
        Request&     getRequestClass() {return (this->_request);};
        ClientState  getClientState() {return (this->_state);};

        void         setFd(int fd) {this->_fd = fd;};
        void         setFlags(int flags) {this->_flags = flags;};
        void         setRequestBuffer(std::string requestBuffer) {this->_requestBuffer = requestBuffer;};
        // void         setResponseBuffer(std::string responseBuffer) {this->_responseBuffer = responseBuffer;};
        void         setReadyToWrite(bool ReadyToWrite) {this->_ReadyToWrite = ReadyToWrite;};
        void         setClientState(ClientState state) {this->_state = state;};
        void         setResponseBuffer(std::deque<std::string> _responseBuffer);

        Response&    getResponseClass(){return _response;}
        void         clearRequest();
        unsigned int getContentLength();
        bool         isUpload();
        // std::vector<unsigned char>& getBuffer(){return _buffer;}
        void         clearResponse();
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
