#include "ConfigFile.hpp"
#include "../utils/utils.hpp"

//sert à vérifier si une chaîne de caractères est une adresse IPv4 valide.
bool ServerConfig::isValidIPv4(const std::string &ip)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 1;
}

bool ServerConfig::isServerStart(const std::string &line)
{
    std::string t = trim(line);
    if (t.rfind("server", 0) != 0)
        return false;
    std::string rest = trim(t.substr(6));
    return (rest == "{");
}

void ServerConfig::applyServersDefaults()
{
    if (!this->_hasRoot)
        this->_root = "./www";
    if (!this->_hasIndex)
        this->_index = "index.html";
    if (!this->_hasServerName)
        this->_serverName = "";
    for (size_t i = 0; i < this->_locations.size(); ++i)
        this->_locations[i].applyLocationDefaults(*this);
}

void ServerConfig::parseServer(std::ifstream &file)
{
    std::string line;

    while (std::getline(file, line))
    {
        std::string t = trim(line);

        if (t == "}")
            break;
        if (t.empty())
            continue;
        if (t.rfind("location ", 0) == 0)
        {
            LocationConfig location;
            location.parseLocationDirective(*this ,file, t);
            this->_locations.push_back(location);
        }
        else
            this->parseServerLine(t);
    }
}

int ServerConfig::createListener()
{
    std::cout << "Creating listener for port:" << _listenPort << std::endl;
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // creation de la socket du server
    if (sockfd == -1)
        throw std::runtime_error("couldn't create socket");
    
     //rajouter des options et permettre de réutiliser adressse
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(sockfd);
        throw std::runtime_error("setsockopt failed");
    }
    
    // setup socket adress parameters 
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(_listenPort);
    if (_listenHost.empty() || _listenHost == "0.0.0.0")
        sockaddr.sin_addr.s_addr = INADDR_ANY;
    else
    {
        if (inet_pton(AF_INET, _listenHost.c_str(), &sockaddr.sin_addr) <= 0)
        {
            close(sockfd);
            throw std::runtime_error("invalid address: " + _listenHost);
        }
    }
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) // on attache les parametre setup a la socket 
    {
        close(sockfd);
        // throw std::runtime_error("couldn't bind on port " + std::to_string(server.listenPort));
    }
    if (listen(sockfd, 100) < 0) // la socket peut mtn accpeter les connections avec une queue d'attente de 5 maximum
    {
        close(sockfd);
        // throw std::runtime_error("couldn't listen on port " + std::to_string(server.listenPort));
    }
    // setNonBlocking(sockfd); // se fait dans la boucle de epoll
    std::cout << "Listener ready on " << _listenHost << ":" << _listenPort << std::endl;
    return sockfd;
}


void ServerConfig::print_attributes()
{
    std::cout << "  Host  = " << _listenHost << std::endl;
    std::cout << "  Port  = " << _listenPort << std::endl;
    std::cout << "  Name  = " << _serverName << std::endl;
    std::cout << "  Root  = " << _root << std::endl;
    std::cout << "  Index = " << _index << std::endl;
    if (!_errorPages.empty())
    {
        std::cout << "  Error Pages:" << std::endl;
        for (std::map<int, std::string>::const_iterator it = _errorPages.begin();
            it != _errorPages.end(); ++it)
            std::cout << "    " << it->first << " -> " << it->second << std::endl;
    }
    else
        std::cout << "  Error Pages: none" << std::endl;

    if (_locations.empty())
        std::cout << "  No locations defined." << std::endl;
    for (size_t i = 0; i < _locations.size(); i++)
    {
        const LocationConfig &loc = _locations[i];
        std::cout << "\n  Location " << i << ":" << std::endl;
        std::cout << "    path      = " << loc.getPath() << std::endl;
        std::cout << "    root      = " << loc.getRoot() << std::endl;
        std::cout << "    index     = " << loc.getIndex() << std::endl;
        std::cout << "    autoindex = " << (loc.getAutoIndex() ? "on" : "off") << std::endl;
        std::cout << "    method allowed = " << std::endl;
        for (size_t i = 0; i < loc.getAllowMethods().size(); i++)
            std::cout << loc.getAllowMethods()[i] << std::endl;
        std::cout << "    max body size =   " << loc.getHasMaxBodySize() << std::endl;
        std::cout << "    cgi Bin   = " << loc.getCgiBin() << std::endl;
        std::cout << "    cgi Ext   = " << loc.getCgiExt() << std::endl;
        std::cout << "   Redirect    = " << loc.getRedirect()<< std::endl;
    }
}
