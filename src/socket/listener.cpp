#include "../../include/webserv.hpp"

//fonction pour socket(), bind(), listen(), etc...
//Listener = socket spécial qui écoute les connexions entrantes (unique rôle)
//socket = point de communication (style une prise)
int createListener(const ServerConfig &server)
{
    std::cout << "Creating listener for port:" << server.listenPort << std::endl;
    
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
    sockaddr.sin_port = htons(server.listenPort);
    if (server.listenHost.empty() || server.listenHost == "0.0.0.0")
        sockaddr.sin_addr.s_addr = INADDR_ANY;
    else
    {
        if (inet_pton(AF_INET, server.listenHost.c_str(), &sockaddr.sin_addr) <= 0)
        {
            close(sockfd);
            throw std::runtime_error("invalid address: " + server.listenHost);
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
    std::cout << "Listener ready on " << server.listenHost << ":" << server.listenPort << std::endl;
    return sockfd;
}
