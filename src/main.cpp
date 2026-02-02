#include "config/Manager.hpp"

int main(int ac, char **av, char **env)
{
    (void)env;
    Manager man;
    try 
    {
        if (ac != 2)
            throw std::runtime_error("usage: ./webserv [configuration file]");
        man.initServers(av[1]);
        // man.print_servers_attributes();
        man.startWebserv();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }
    return (0);
}
