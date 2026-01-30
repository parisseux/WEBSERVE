#include "ConfigFile.hpp"

//sert à vérifier si une chaîne de caractères est une adresse IPv4 valide.
bool isValidIPv4(const std::string &ip)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 1;
}

std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

bool isServerStart(const std::string &line)
{
    std::string t = trim(line);
    if (t.rfind("server", 0) != 0)
        return false;
    std::string rest = trim(t.substr(6));
    return (rest == "{");
}

std::string removeSemicolon(const std::string &s)
{
    std::string out = trim(s);
    if (!out.empty() && out[out.size() - 1] == ';')
        out.erase(out.size() - 1);
    return trim(out);
}

static void applyLocationDefaults(LocationConfig &loca, ServerConfig &server)
{
    if (!loca.getHasAutoIndex())
        loca.setAutoIndex(false);
    if (!loca.getHasAllowMethods())
        loca.setAllowMethods().push_back("GET");
    if (!loca.getHasIndex())
        loca.setIndex(server.getIndex());
    if (!loca.getHasRoot())
        loca.setRoot(server.getRoot());
}

void ServerConfig::applyServersDefaults()
{
    if (!this->_hasRoot)
        server.getRoot() = "./www";
    if (!server.getHasIndex())
        server.getIndex() = "index.html";
    if (!server.getHasServerName())
        server.getServerName() = "";
    for (size_t i = 0; i < server.getLocations().size(); ++i)
        applyLocationDefaults(server.getLocations()[i], server);
}
void ServerConfig::parseServer(std::ifstream &file)
{
    ServerConfig server;
    std::string line;

    while (std::getline(file, line))
    {
        std::string t = trim(line);

        if (t == "}")
            break;
        if (t.empty())
            continue;
        if (t.rfind("location ", 0) == 0)
            parseLocationDirective(server, file, t);
        else
            parseServerLine(server, t);
    }
    // return server;
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