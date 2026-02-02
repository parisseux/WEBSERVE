#ifndef MANAGER_HPP
# define MANAGER_HPP

#include "ConfigFile.hpp"
#include "../socket/epoll.hpp"

class Manager
{
    private:
        std::vector<ServerConfig> _servers;
        std::vector<int> _listener_fds;
        Epoll _epoll;
    public:
        Manager() {std::cout << "Manager constructor called" << std::endl;};
        ~Manager() {std::cout << "Manager destructor called" << std::endl;};
        void        startWebserv();
        void        initServers(const std::string &configFile);
        void        print_servers_attributes();
};

#endif