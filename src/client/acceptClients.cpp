#include "../../include/webserv.hpp"

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
