#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <iostream>
# include <unistd.h>
# include <cstdlib>

# include "../request/Request.hpp"
# define MAXBYTE 1024

Request basicSocket();
int createListener(const ServerConfig &server);
void setNonBlocking(int fd);

#endif