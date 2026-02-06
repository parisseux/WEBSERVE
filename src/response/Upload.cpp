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
    std::string boundaryStr = ct.substr(pos + 9);
    _boundary.assign(boundaryStr.begin(), boundaryStr.end());
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

std::map<std::string, std::string> Upload::FillHeaders(std::string headerStr)
{
    std::map<std::string, std::string> headers; 
    //permet de transfomrer string en flux comme si cetait un fichier donc on peut utilsier getline
    std::stringstream ss(headerStr);
    std::string line;

    while (std::getline(ss, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            break;
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos)
            continue;
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        if (!value.empty() && value[0] == ' ')
            value.erase(0, 1);
        headers[key] = value;
    }
    return (headers);
}

void Upload::ParseBody(const Request &req)
{
    const std::vector<unsigned char> &body = req.getBodyBinary();
    size_t pos = 0;
    
    while (1)
    {
        //chercher boudary si on a arrive a la fin de vecteur sans trouver break

        //si on est au deernier boudray on break

        //
        // auto it = std::find(body.begin() + pos, body.end(),_boundary.begin(), _boundary.end());
        // if (it == body.end())
        //     break;
        // if (body.compare(boundary_pos, _boundary.size() + 2, _boundary + "--") == 0)
        //     break;
        // // +2 pour -- et +2 pour \r\n
        // pos = boundary_pos + _boundary.size() + 2 + 2;
        // size_t headerEnd_pos = body.find("\r\n\r\n", pos);
        // //la string contient toute la partie headers que je dois mnt remettre dans headers
        // std::string headersStr = body.substr(pos, headerEnd_pos - pos);

        // pos += headerEnd_pos + 4;
        // size_t contentEnd_pos = body.find(_boundary, pos);
        // std::string contentStr = body.substr(pos, contentEnd_pos - pos);
        
        // //on crer la partie avec headers et content et on l-ajoute au vecteur
        // Part p;
        // p.content = std::move(content); // vecteur binaire
        // p.headers = FillHeaders(headersStr);
        // _parts.push_back(std::move(p));
    }
    
    std::cout << "Parsing of body finish" << std::endl;

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
    
    //ici je viens parser le body pour le "cut" en "parts" et contenir ces parties dans un vecteur pour pouvoir les traiter apres
    ParseBody(req);

    return (Response::Error(42, "finir fonction"));
    // Response res;
    //return (res.buildUploadResponse(file))
}