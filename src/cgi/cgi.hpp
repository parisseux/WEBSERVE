#ifndef CGI_HPP
# define CGI_HPP

# include <string>
# include <vector>
# include <map>

class Client;
class Epoll;
class Response;
class ServerConfig;
class LocationConfig;
class Request;


class Cgi
{
    private:
        std::string _path;
        std::vector<std::string> _envCgiString;
        std::vector<char*> _envCgi;       

    public:
        Response handleCgi(Request &req, const ServerConfig &server, const LocationConfig &loc, std::map<int, Cgi*> &_CgiMap, Client *client, Epoll &epoll);
        void readFd(int fd, std::string &content);
        void addCgiEnv(Request &req, std::string path, std::vector<std::string> &envCgiString);
        void MakeCgiEnv(Request &req);
        std::string GetEffectiveRoot(const ServerConfig &server, const LocationConfig &loc);
        std::string GetRelativPath(const std::string &reqPath, const std::string &locPath);
        std::string JoinPath(const std::string &root, const std::string &relativPath);

};
    bool isCgi(const Request &req, const ServerConfig &server, const LocationConfig &loc);

#endif