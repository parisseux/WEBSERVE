#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>
# include <vector>
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
        // std::vector<unsigned char> _buffer;
    public:
        Client() {std::cout << "Client constructor called" << std::endl;};
        ~Client() {std::cout << "Client destructor called" << std::endl;};

        int&         getFd() {return (this->_fd);};
        int&         getFlags() {return (this->_flags);};
        std::string& getRequestBuffer() {return (this->_requestBuffer);};
        std::string& getResponseBuffer() {return (this->_responseBuffer);};
        bool&        getReadyToWrite() {return (this->_ReadyToWrite);};
        Request&     getRequestClass() {return (this->_request);};
        ClientState  getClientState() {return (this->_state);};

        void         setFd(int fd) {this->_fd = fd;};
        void         setFlags(int flags) {this->_flags = flags;};
        void         setRequestBuffer(std::string requestBuffer) {this->_requestBuffer = requestBuffer;};
        void         setResponseBuffer(std::string responseBuffer) {this->_responseBuffer = responseBuffer;};
        void         setReadyToWrite(bool ReadyToWrite) {this->_ReadyToWrite = ReadyToWrite;};
        void         setClientState(ClientState state) {this->_state = state;};

        void         clearRequest();
        unsigned int getContentLength();
        bool         isUpload();
        // std::vector<unsigned char>& getBuffer(){return _buffer;}
};

void setNonBlocking(int fd);

#endif
