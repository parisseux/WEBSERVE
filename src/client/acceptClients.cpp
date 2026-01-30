#include "client.hpp"

//obligatoire pour select() sinon les clients attendent tant que ça bloque 
//pour le premier client
//Par défaut une socket est blocante 
void setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("fcntl F_GETFL failed");

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl F_SETFL failed");
}

int acceptNewClient(int listenerFd)
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    int clientfd = accept(listenerFd, (sockaddr*)&addr, &len);
    if (clientfd < 0)
        return -1;

    setNonBlocking(clientfd);
    return clientfd;
}
