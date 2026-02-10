#ifndef UPLOAD_HPP
# define UPLOAD_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <iostream>
# include <fstream>
# include <vector>
# include <algorithm>
# include "../response/Response.hpp"
# include "../config/LocationConfig.hpp"

class LocationConfig;
class Request;
class Response;

struct Part {
    std::vector<unsigned char> content;
    std::map<std::string, std::string> headers;
};

class Upload {
private:
    std::string _uploadDir;
    std::vector<unsigned char> _boundary;
    std::vector<Part> _parts;
    std::vector<std::string> _uploadedFiles;
    int checkHeader(const LocationConfig &loc, const Request &req);
    bool parseBoundary(const Request &req);
    bool canWrite(const std::string &path);
    bool dirExists(const std::string &path);
    Response buildResponse(const std::string file);
    void ParseBody(const Request &req);
    std::map<std::string, std::string> FillHeaders(std::string headerStr);
    void ProcessParts();

public:
    int CheckBodySize(const LocationConfig &loc, const Request &req);
    Response Handle(const LocationConfig &loc, const Request &req);
};


#endif 