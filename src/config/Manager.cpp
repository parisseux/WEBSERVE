#include "Manager.hpp"
#include "ConfigFile.hpp"

void Manager::startWebserv()
{
    //créer sockets d'écoute
    for (size_t i = 0; i < _servers.size(); i++)
        _listener_fds.push_back(_servers[i].createListener());
    
    //lancer boucle principale
    std::cout << "Lancement de la boucle principale" << std::endl;
    _epoll.epollManagment(_listener_fds, _servers);

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

    while (std::getline(file, line))
    {
        ServerConfig config;
        if (!config.isServerStart(line))
            continue;
        config.parseServer(file);

        if (!config.getHasListen())
            throw std::runtime_error("missing listen directive");

        config.applyServersDefaults();
        this->_servers.push_back(config);
        print_servers_attributes();
    }
    if (this->_servers.empty())
        throw std::runtime_error("no valid server block found in config");
}

void Manager::print_servers_attributes()
{
    for (size_t i = 0; i < _servers.size(); ++i)
        _servers[i].print_attributes();
}
