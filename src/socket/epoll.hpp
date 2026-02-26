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

class Request;
class Client;
class Cgi;

class Epoll
{
    private:
        int                     _epFd;
        int                     _eventWait;
        struct  epoll_event     _ev;
        struct  epoll_event     _events[10];
        std::map<int, Client*>  _clientsMap;
        std::map<int, Cgi*>     _CgiMap;
		std::map<int, Client*>::iterator _it;
		Client *_client;      
		bool _is_listener;
	    bool _isCgi;                   
    public:
        Epoll() {std::cout << "Epoll constructor called" << std::endl;};
        ~Epoll() {std::cout << "Epoll destructor called" << std::endl;};

        int&    getEpFd() {return (this->_epFd);};
        int&    getEventWait() {return (this->_eventWait);};

        void    setEpFd(int epFd) {this->_epFd = epFd;};
        void    setEventWait(int eventWait) {this->_eventWait = eventWait;};

        epoll_event* getEvent(){return &_ev;}        
        void setEvent(uint32_t POLL){_ev.events = POLL;}
        void setEventFd(int fd){_ev.data.fd = fd;}
        void epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> &servers);
        void creatEpollFdListeners(std::vector<int>& listener_fds);        
        void manageClientRequest(Client *client, ssize_t byteReads, char *buf);
        void manageCgi(Client *client, int byteReads, char *buf);
        void creactNewClient(std::vector<int>& listener_fds, int j);
        void HeaderEnd(Client *client);
        void NewClientConnection(std::vector<int>& listener_fds, int eventFd);
        void MatchEventWithClient(int eventFd);
        void HandleEpollin(int eventFd);
        void HandleEpollout();
        void closeCgiFd();
        void generatePendingResponse(std::vector<ServerConfig> &servers);
        void formatingchunk(Client *client, std::string bufferString); 
        void printClientMap();
};

#endif
