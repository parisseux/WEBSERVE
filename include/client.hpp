#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>

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
};

struct Epoll
{
    public:
    // private:
        int                 _ep_fd;
        int                 _event_wait;
        struct  epoll_event _ev;
        struct  epoll_event _events[10];
    // public:
        Epoll();
        ~Epoll();
        void    setEpFd(int ep_fd);
        void    setEventWait(int event_wait);
        int&    getEpFd();
        int&    getEventWait();
};

void epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> servers);

#endif
