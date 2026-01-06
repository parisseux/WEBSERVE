
#ifndef SOCKET_CALC_HPP
# define SOCKET_CALC_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <iostream>
# include <unistd.h>
# include <cstdlib>
#include "../include/request/request.hpp"
# define MAXBYTE 1024

request basic_socket();
int createListener(const ServerConfig &server);
void setNonBlocking(int fd);

#endif