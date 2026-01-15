
#include "../include/webserv.hpp"

//partie DAVID
static void eventLoop(std::vector<ServerConfig> servers, const std::vector<int>& listeners)
{
    std::map<int, Client*> clients_map;

    while (true)
    {
        fd_set read_fds;
        fd_set write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        int max_fd = 0;
        
    }
}

static void startWebserv(std::vector<ServerConfig> servers)
{
    //créer sockets d'écoute
    std::vector<int> listener_fds;
    for (size_t i = 0; i < servers.size(); i++)
    {
        int listener_fd = createListener(servers[i]);
        listener_fds.push_back(listener_fd);
    }

    //lancer boucle principale
    std::cout << "Lancement de la boucle principale" << std::endl;
    epoll_managment(listener_fd, clients_map);

    //fermer les sockets d'écoute
    for (size_t i = 0; i < listener_fds.size(); i++)
        close(listener_fds.at(i));
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
