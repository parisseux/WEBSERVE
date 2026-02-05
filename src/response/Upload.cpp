#include "Upload.hpp"

bool Upload::dirExists(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return (false);
    return ((info.st_mode & S_IFDIR) != 0);
}

bool Upload::canWrite(const std::string &path)
{
    return (access(path.c_str(), W_OK) == 0);
}

bool Upload::parseBoundary(const Request &req)
{    
    std::string ct = req.getHeader("Content-Type");
    size_t pos = ct.find("boundary=");
    if (pos == std::string::npos)
        return false;
    _boundary = ct.substr(pos + 9);
    return true;
}

int Upload::checkHeader(const LocationConfig &loc, const Request &req)
{
    if (!parseBoundary(req))
        return 400;
    std::cout << "good detection of boundary" << std::endl;
    std::string cl = req.getHeader("Content-Length");
    cl.erase(cl.find_last_not_of(" \r\n") + 1);
    std::cout << "Content length: " << cl << std::endl;
    if (cl.empty())
        return 400;
    std::cout << "good detection of contente length" << std::endl;
    char *endptr;
    long length = std::strtol(cl.c_str(), &endptr, 10);
    if (*endptr != '\0')
    {
        std::cout << "incorrect content lenght" << std::endl;
        return 400;
    }   
    if (length <= 0)
    {
        std::cout << "length is 0 or negativ" << std::endl;
        return 400;
    }
    if (length > static_cast<long>(loc.getMaxBodySize()))
    {
        std::cout << "body size is too big, max body autorized : " << loc.getMaxBodySize() << std::endl;
        return 413;
    }
    return 200;
}

int Upload::CheckBodySize(const LocationConfig &loc, const Request &req)
{
    if (!loc.getHasMaxBodySize())
        return 0;
    std::cout << "Body size: " << req.getBody().size() << " Size autorized: " << loc.getMaxBodySize() << std::endl;
    if (req.getBody().size() > loc.getMaxBodySize())
        return 413;
    return (0);
}

Response Upload::Handle(const LocationConfig &loc, const Request &req)
{
    if (!loc.getHasUploadPath())
        return (Response::Error(400, "Bad Request"));
    std::cout << "good uplaod file" << std::endl;
    _uploadDir = loc.getUploadPath();
    int status = checkHeader(loc, req);
    if (status == 400)
        return (Response::Error(400, "Bad Request"));
    if (status == 413)
        return (Response::Error(413, "Request entity too large"));
    std::cout << "good headers" << std::endl;  
    
    if (!dirExists(_uploadDir))
        return Response::Error(500, "Upload folder does not exist");
    if (!canWrite(_uploadDir))
        return Response::Error(403, "No write permission in upload folder");

    std::cout << "--------- Handling upload ------" << std::endl;
    std::cout << "Boundary : " << _boundary << std::endl;
    

    return (Response::Error(42, "finir fonction"));
    // Response res;
    //return (res.buildUploadResponse(file))
}