#ifndef CGI_HPP
# define CGI_HPP
# include "response.hpp"

bool isCgi(const Request &req, const ServerConfig &server, const LocationConfig &loc);
Response handleCgi(const Request &req, const ServerConfig &server, const LocationConfig &loc);

#endif