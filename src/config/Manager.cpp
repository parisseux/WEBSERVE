#include "Manager.hpp"
#include "ConfigFile.hpp"
# include "../../include/webserv.hpp"

void Manager::startWebserv()
{
    //créer sockets d'écoute
    for (size_t i = 0; i < _servers.size(); i++)
    {
        _listener_fds.push_back(_servers[i].createListener());
        // int listener_fd = createListener(this->servers[i]);
        // listener_fds.push_back(listener_fd);
    }
    
    //lancer boucle principale
    std::cout << "Lancement de la boucle principale" << std::endl;
    epollManagment(_listener_fds, _servers);

    //fermer les sockets d'écoute
    for (size_t i = 0; i < _listener_fds.size(); i++)
        close(_listener_fds.at(i));
}

void Manager::initServers(const std::string &configFile)
{
    std::ifstream file(configFile.c_str());
    if (!file.is_open())
        throw std::runtime_error("could not open config file");
    this->_servers.clear();
    std::string line;
    size_t serverCount = 0;
    std::cout << "Test" << std::endl;
    while (std::getline(file, line))
    {
        if (this->_servers[serverCount].isServerStart(line))
            ++serverCount;
        try
        {
            ServerConfig config;
            this->_servers.push_back(config);
            this->_servers[serverCount].parseServer(file);
            // servers[serverCount] = parseServer(file);
            std::cout << "Test" << std::endl;
            if (!this->_servers[serverCount].getHasListen())
                throw std::runtime_error("missing listen directive");
            std::cout << "Test" << std::endl;
            this->_servers[serverCount].applyServersDefaults();
            // applyServersDefaults(servers[serverCount]);
            std::cout << "Test" << std::endl;
            // servers.push_back(server);
        }
        catch (const std::exception& e) 
        {
            //throw std::runtime_error("config error in server block #" + std::to_string(serverCount) + ": " + e.what());
        }
    }
    if (this->_servers.empty())
        throw std::runtime_error("no valid server block found in config");
}
