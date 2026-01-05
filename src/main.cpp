
#include "../include/webserv.hpp"

void print_servers_attributes(std::vector<ServerConfig> &servers)
{
    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerConfig &server = servers[i];
        std::cout << "Parsed server: " << i + 1<< std::endl;
        std::cout << "  Host  = " << server.listenHost << std::endl;
        std::cout << "  Port  = " << server.listenPort << std::endl;
        std::cout << "  Name  = " << server.serverName << std::endl;
        std::cout << "  Root  = " << server.root << std::endl;
        std::cout << "  Index = " << server.index << std::endl;
        if (!server.errorPages.empty())
        {
            std::cout << "  Error Pages:" << std::endl;
            for (std::map<int, std::string>::const_iterator it = server.errorPages.begin();
                it != server.errorPages.end(); ++it)
                std::cout << "    " << it->first << " -> " << it->second << std::endl;
        }
        else
            std::cout << "  Error Pages: none" << std::endl;

        if (server.locations.empty())
            std::cout << "  No locations defined." << std::endl;

        for (size_t i = 0; i < server.locations.size(); i++)
        {
            const LocationConfig &loc = server.locations[i];
            std::cout << "\n  Location " << i << ":" << std::endl;
            std::cout << "    path      = " << loc.path << std::endl;
            std::cout << "    root      = " << loc.root << std::endl;
            std::cout << "    index     = " << loc.index << std::endl;
            std::cout << "    autoindex = " << (loc.autoindex ? "on" : "off") << std::endl;
        }
    }
}

void eventLoop()//listeners, clients, servers
{

    //attendre evenements (Select)
    // WaitForEvents();

    // if (listenerReady)
    //     acceptNewClient

    // if (clientReadyToRead)
    // {
    //     readFromClient();

        request Request = basic_socket();
        Request.display_request();
//         Res = handleRequest();
// //}
//     if (clientReadyToWrite)
//     {
//         writeToClient();
//     }
}

void startWebserv(std::vector<ServerConfig> servers)
{
    //créer sockets d'écoute 
    std::cout << "Setup Listeners" << std::endl;

    //map pour stocker les clients connectés
    std::cout << "créer structure pour clients connectés" << std::endl;
    (void)servers;
    //lancer boucle principale
    eventLoop();
}

int mess_error(std::string mess, int exit_code)
{
    std::cerr << "Error: " << mess << std::endl;
    return exit_code;
}


int main(int ac, char **av)
{
    if (ac != 2)
        return(mess_error("usage: ./webserv [configuration file]", 1));
    std::vector<ServerConfig> servers;
    if (!initServers(av[1], servers))
        return(mess_error("server initialisation failed.", 1));
    //vous pouvez mettre en commentaire cest juste pour du debug de PARSING
    //print_servers_attributes(servers);
    std::cout << "Lets start webserv!!!!!" <<std::endl;    
    startWebserv(servers);
    return (0);
}
