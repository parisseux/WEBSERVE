#ifndef UPLOAD_HPP
# define UPLOAD_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <iostream>
# include <fstream>
# include "../response/Response.hpp"
# include "../config/LocationConfig.hpp"

class LocationConfig;
class Request;
class Response;

struct Part
{
    std::map<std::string, std::string> headers;
    std::string content;
};

class Upload {
private:
    std::string _uploadDir;
    std::string _boundary;
    std::vector<Part> parts;
    int checkHeader(const LocationConfig &loc, const Request &req);
    bool parseBoundary(const Request &req);
    bool canWrite(const std::string &path);
    bool dirExists(const std::string &path);
    Response buildResponse(const std::string file);

public:
    int CheckBodySize(const LocationConfig &loc, const Request &req);
    Response Handle(const LocationConfig &loc, const Request &req);
};


#endif 