#ifndef STATICTARGET_HPP
#define STATICTARGET_HPP

#include "webserv.hpp"

class StaticTarget
{
private:
    static std::string getEffectiveRoot(const ServerConfig &server,const LocationConfig &loc);
    static std::string getRelativPath(const std::string &reqPath,const std::string &locPath);
    static std::string joinPath(const std::string &root, const std::string &relativPath);
    static bool readFile(const std::string &path, std::string &content);
    static std::string getContentType(const std::string &path);

public:
    static ResolvedTarget resolve(const Request &req, const ServerConfig &server, const LocationConfig &loc);
    static Response buildResponse(const Request &req, const ResolvedTarget &target);
};

#endif
