#ifndef EPOLL_HPP
# define EPOLL_HPP 

# include <map>
# include <vector>
# include <fcntl.h>
# include <unistd.h>
# include <sys/epoll.h>
# include "../client/client.hpp"
# include "../config/config.hpp"
# include "../response/response.hpp"
# include "../request/request.hpp"


class Epoll
{
    private:
        int                 _ep_fd;
        int                 _event_wait;
        struct  epoll_event _ev;
        struct  epoll_event _events[10];
        std::map<int, Client*> Clients_map;        
    public:
        Epoll();
        ~Epoll();
        void    setEpFd(int ep_fd);
        void    setEventWait(int event_wait);
        int&    getEpFd();
        int&    getEventWait();
        void epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> servers);
        void creatEpollFdListeners(std::vector<int>& listener_fds);        
        void manageClientRequest(Client *client, int byteReads, char *buf, std::vector<ServerConfig> servers);
        void creactNewClient(std::vector<int>& listener_fds, int j);
        void HeaderEnd(Client *client, std::string bufferString);
};

#endif