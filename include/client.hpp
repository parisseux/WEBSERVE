#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>

# define MAX_PENDING 20

class Client 
{
    private:
        int         _fd;
        int         _flags;
        std::string _requestBuffer;
        std::string _responseBuffer;
        bool        _ReadyToWrite;    
    public:
        request     _request;    
        void set_fd(int fd);
        void set_flags(int flags);
        void set_requestBuffer(std::string requestBuffer);
        void set_responseBuffer(std::string _responseBuffer);
        void set_ReadyToWrite(bool ReadyToWrite);
        int         get_fd();
        int         get_flags();
        std::string get_requestBuffer();
        std::string get_responseBuffer();
        bool        get_ReadyToWrite(); 
        Client();
        ~Client();
};

struct Epoll
{
    int     ep_fd;
    int     event_wait;
    struct  epoll_event ev;
    struct  epoll_event events[10];
};

void epoll_managment (int listener_fd, std::map<int, Client*>& Clients_map);

#endif
