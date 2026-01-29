#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

# include "../../include/webserv.hpp"
# include "handleUpload.hpp"
# include "staticTarget.hpp"

class RequestHandler
{
private:
    Request& _req;
    const std::vector<LocationConfig>& _locations;
    const ServerConfig& _server;
    const LocationConfig* _loc;
    int validateRequest() const;
    int methodAllowed() const;
    int checkBodySize() const;
    const LocationConfig* matchLocation() const;
    bool startsWith(const std::string& s, const std::string& prefix) const;
public:
    RequestHandler(Request& req, const std::vector<LocationConfig>& locations, const ServerConfig& server);
    Response handle();
};

#endif