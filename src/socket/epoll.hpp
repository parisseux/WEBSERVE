#ifndef EPOLL_HPP
# define EPOLL_HPP 

# include <map>
# include <vector>
# include <fcntl.h>
# include <unistd.h>
# include <sys/epoll.h>
# include "../client/client.hpp"
# include "../config/ConfigFile.hpp"
# include "../response/Response.hpp"
# include "../request/Request.hpp"
# include "../cgi/cgi.hpp"


class Epoll
{
    private:
        int                     _epFd;
        int                     _eventWait;
        struct  epoll_event     _ev;
        struct  epoll_event     _events[10];
        std::map<int, Client*>  _clientsMap;
        std::map<int, Cgi*>     _CgiMap;
    public:
        Epoll() {std::cout << "Epoll constructor called" << std::endl;};
        ~Epoll() {std::cout << "Epoll destructor called" << std::endl;};

        int&    getEpFd() {return (this->_epFd);};
        int&    getEventWait() {return (this->_eventWait);};

        void    setEpFd(int epFd) {this->_epFd = epFd;};
        void    setEventWait(int eventWait) {this->_eventWait = eventWait;};

        void epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> &servers);
        void creatEpollFdListeners(std::vector<int>& listener_fds);        
        void manageClientRequest(Client *client, int byteReads, char *buf, std::vector<ServerConfig> &servers, std::map<int, Cgi*> &_CgiMap);
        void creactNewClient(std::vector<int>& listener_fds, int j);
        void HeaderEnd(Client *client, std::string bufferString);
};

#endif
