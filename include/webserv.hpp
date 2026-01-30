#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "config.hpp"
# include "utils.hpp"
# include "socket.hpp"
# include "../src/request/Request.hpp"
# include "client.hpp"
# include "../src/response/Response.hpp"
# include "../src/response/StaticTarget.hpp"

# include <utility>
# include <sys/epoll.h>
# include <fcntl.h>
# include <stdexcept>

//Response HandleRequest(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server);
// Response HandleRequest(const Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server);

#endif
