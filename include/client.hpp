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
        void         set_fd(int fd);
        void         set_flags(int flags);
        void         set_requestBuffer(std::string requestBuffer);
        void         set_responseBuffer(std::string _responseBuffer);
        void         set_ReadyToWrite(bool ReadyToWrite);
        void         set_clientState(ClientState state); 
        int&         get_fd();
        int&         get_flags();
        std::string& get_requestBuffer();
        std::string& get_responseBuffer();
        bool&        get_ReadyToWrite();
        Request&     get_requestClass();
        ClientState  get_clientState();
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
        void    set_epfd(int ep_fd);
        void    set_event_wait(int event_wait);
        int&    get_epfd();
        int&    get_event_wait();
};

void epoll_managment (std::vector<int>& listener_fds, std::map<int, Client*>& Clients_map, std::vector<ServerConfig> servers);

#endif
