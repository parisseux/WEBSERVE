#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>

# define MAX_PENDING 20

class Client 
{
    public:
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
        int         _fd;
        int         _flags;
        request     _request;
        std::string _requestBuffer;
        std::string _responseBuffer;
        bool        _ReadyToWrite;
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

#endif
