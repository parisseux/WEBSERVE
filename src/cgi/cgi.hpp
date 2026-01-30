#ifndef CGI_HPP
# define CGI_HPP
# include "../response/response.hpp"
# include "unistd.h"

class cgi
{
    private:

    public:
        Response handleCgi(Request &req, const ServerConfig &server, const LocationConfig &loc);
        void readFd(int fd, std::string &content);
        void makeCgiEnv(Request &req, std::string path, char **cgiEnv);
        void makeCgi(Request &req, std::string path, std::vector<std::string> &envCgiString);        
};
        bool isCgi(const Request &req, const ServerConfig &server, const LocationConfig &loc);





#endif