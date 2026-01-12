#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>

# define MAX_PENDING 20

class Client 
{
    int fd;
    int flags;
    std::string requestBuffer;
    std::string responseBuffer;
    bool ReadyToWrite;
    Client(): ReadyToWrite(false) {}
};

struct Epoll
{
    int     ep_fd;
    int     event_wait;
    struct  epoll_event ev;
    struct  epoll_event events[10];
};

#endif