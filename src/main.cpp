#include "../include/webserv.hpp"
#include "../config/Manager.hpp"
# include "config/config.hpp"
# include "utils/utils.hpp"
# include "socket/socket.hpp"
# include "request/Request.hpp"
# include "response/Response.hpp"
# include "client/client.hpp"
# include "cgi/cgi.hpp"
# include "socket/epoll.hpp"

# include <utility>
# include <sys/epoll.h>
# include <fcntl.h>
# include <stdexcept>
# include <sys/wait.h>
# include <unistd.h>

// Response HandleRequest(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server);

static void startWebserv(std::vector<ServerConfig> servers)
{
    //créer sockets d'écoute
    std::vector<int> listener_fds;
    for (size_t i = 0; i < servers.size(); i++)
    {
        int listener_fd = createListener(servers[i]);
        listener_fds.push_back(listener_fd);
    }
    Epoll epoll;
    
    //lancer boucle principale
    std::cout << "Lancement de la boucle principale" << std::endl;
    epoll.epollManagment(listener_fds, servers);

    //fermer les sockets d'écoute
    for (size_t i = 0; i < listener_fds.size(); i++)
        close(listener_fds.at(i));
}


int main(int ac, char **av, char **env)
{
    Manager man;
    try 
    {
        if (ac != 2)
            throw std::runtime_error("usage: ./webserv [configuration file]");
        man.initServers(av[1]);
        // print_servers_attributes(servers);
        man.startWebserv();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }
    return (0);
}
