#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "config.hpp"
# include "utils/utils_1.hpp"
# include "socket/basic_socket.hpp"
# include "request/request.hpp"
# include "client.hpp"
# include <utility>
# include "response.hpp"

#include <fcntl.h>
#include <stdexcept>
Response HandleRequest(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server);

#endif
