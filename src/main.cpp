#include "../include/webserv.hpp"
#include "../config/Manager.hpp"

int main(int ac, char **av)
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
