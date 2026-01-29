#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "config.hpp"
# include "utils.hpp"
# include "socket.hpp"
# include "request.hpp"
# include "response.hpp"
# include "client.hpp"
# include "cgi.hpp"

# include <utility>
# include <sys/epoll.h>
# include <fcntl.h>
# include <stdexcept>

Response HandleRequest(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server);
// Response HandleRequest(const Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server);

#endif
