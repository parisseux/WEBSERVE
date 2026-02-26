#ifndef STATICTARGET_HPP
# define STATICTARGET_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <iostream>
# include <fstream>
# include "../response/Response.hpp"

class Request;
class Response;
struct ResolvedTarget;
class ServerConfig;
class LocationConfig;
class Client;

class StaticTarget {
private:
    std::string getContentType(const std::string& path);
    bool ReadFile(const std::string &path, std::string &content, Client *client);
    std::string JoinPath(const std::string &root, const std::string &relativPath);
    std::string GetRelativPath(const std::string &reqPath);
    std::string GetEffectiveRoot(const ServerConfig &server, const LocationConfig &loc);
public:
    void BuildStaticResponse(const Request& req, const ResolvedTarget& target, Client *client, Response &res);
    ResolvedTarget ResolveStaticTarget(const Request &req, const ServerConfig &server, const LocationConfig &loc);
};

#endif