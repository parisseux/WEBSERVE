
#include "../include/webserv.hpp"

static void startWebserv(ServerConfig &servers)
{
    int listener_fd = createListener(servers);
    std::map<int, Client*> clients_map; // a mettre surment dans la struct Serverconfig

    std::cout << "Lancement de la boucle principale" << std::endl;
    epoll_managment(listener_fd, clients_map);

    //fermer les sockets d'écoute 
    // for (size_t i = 0; i < listener_fd.size(); i++)
     close(listener_fd);
}

int main(int ac, char **av)
{
    (void)av;
    (void)ac;
    try 
    {
        // if (ac != 2)
        //     throw std::runtime_error("usage: ./webserv [configuration file]");
        // version avec ecoute seulement sur un port sans parsing du config file.
        ServerConfig servers;
        servers.listenPort = 7070; // setup par defaut d'un port
        // initServers(av[1], servers);
        //print_servers_attributes(servers);
        startWebserv(servers);
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }
    return (0);
}
