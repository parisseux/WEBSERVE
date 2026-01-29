#ifndef HANDLEUPLOAD_HPP
# define HANDLEUPLOAD_HPP

# include "webserv.hpp"
# include <cstdlib>
# include <string>

struct Part {
    std::string name;
    std::string filename;
    std::map<std::string,std::string> headers;
    std::string content;
};

class HandleUpload
{
private:
    std::string _boundary;
    std::string _uploadDir;
    std::vector<Part> _parts;
    bool dirExists(const std::string &path);
    bool canWrite(const std::string &path);
    bool parseBoundary(const Request &req);

public:
    HandleUpload() {}
    ~HandleUpload() {}
    int checkHeaders(const Request &req, const LocationConfig &loc);
    Response handle(const Request &req, const LocationConfig &loc);
    void parseBody(const Request &req);
    void writeFiles();
};

#endif