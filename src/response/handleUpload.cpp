#include "handleUpload.hpp"

bool HandleUpload::dirExists(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR) != 0;
}

bool HandleUpload::canWrite(const std::string &path)
{
    return access(path.c_str(), W_OK) == 0;
}

bool HandleUpload::parseBoundary(const Request &req)
{
    std::string ct = req.getHeader("Content-Type");
    size_t pos = ct.find("boundary=");
    if (pos == std::string::npos)
        return false;
    _boundary = ct.substr(pos + 9);
    return true;
}

int HandleUpload::checkHeaders(const Request &req, const LocationConfig &loc)
{
    if (!parseBoundary(req))
        return 400;
    std::string cl = req.getHeader("Content-Length");
    if (cl.empty())
        return 400;
    char *endptr;
    long length = std::strtol(cl.c_str(), &endptr, 10);
    if (*endptr != '\0')
        return 400;
    if (length <= 0)
        return 400;
    if (length > static_cast<long>(loc._maxBodySize))
        return 413;
    return 200;
}

// void HandleUpload::parseBody(const Request &req)
// {
//     std::string body = req.getBody();
//     std::string fullBoundary = "--" + _boundary;
//     size_t pos = 0;
//     size_t next;

//     while ((next = body.find(fullBoundary, pos)) != std::string::npos) 
//     {
//         size_t start = pos;
//         pos = next + fullBoundary.length();
//         if (body.substr(pos, 2) == "--")
//             break;
//         std::string partString = body.substr(start, next - start);
//         size_t headerEnd = partString.find("\r\n\r\n");
//         std::string headerText = partString.substr(0, headerEnd);
//         std::string content = partString.substr(headerEnd + 4);

//         Part part;
//         part.content = content;


//     }
//     // découper le body avec _boundary
//     // identifier les fichiers
// }

// void HandleUpload::writeFiles()
// {

// }

Response HandleUpload::handle(const Request &req, const LocationConfig &loc)
{

    std::cout << "lala" << std::endl;
    if (!loc.hasUploadPath)
        return (Response::Error(400, "Bad Request"));
    _uploadDir = loc.upload_path;

    int status = checkHeaders(req, loc);
    if (status == 400)
        return Response::Error(400, "Bad Request");
    if (status == 413)
        return Response::Error(413, "Request entity too large");

    if (!dirExists(_uploadDir))
        return Response::Error(500, "Upload folder does not exist");
    if (!canWrite(_uploadDir))
        return Response::Error(403, "No write permission in upload folder");

    // parseBody(req);
    // writeFiles();

    return Response::Error(42, "finir fonction ");
    // return Response::Success("File uploaded successfully");
}



//Upload handler --> permet décrire sur le disque 
//en gros on prend des données envoyé par le client et on les écrit sur le server
//exemple: upload d'image, formulaire (style login avec password), fichier texte, data "POST"
//ici on traite: méthode "POST", body HTTP, 
//réponse possible:
    //201 (uplaod réussi)
    //400 (pas de body)
    //413 taille trop grande
    //405 méthode intédite
/*
POST multipart
   ↓
vérifs headers
   ↓
extraire boundary
   ↓
split body
   ↓
trouver filename
   ↓
écrire fichier
   ↓
réponse HTTP
*/