#include "Upload.hpp"

#include <iostream>
#include <iomanip>
#include <cctype>

// fonction de debug pour voir les parts
void Upload::printParts() const
{
    std::cout << "===== PRINTING PARTS =====" << std::endl;

    for (size_t i = 0; i < _parts.size(); ++i)
    {
        const Part &p = _parts[i];

        std::cout << "\n--- Part " << i + 1 << " ---" << std::endl;

        // 🔹 Print headers
        std::cout << "[Headers]" << std::endl;
        for (std::map<std::string, std::string>::const_iterator it = p.headers.begin();
             it != p.headers.end(); ++it)
        {
            std::cout << it->first << ": " << it->second << std::endl;
        }

        // 🔹 Print content size
        std::cout << "\n[Content size] "
                  << p.content.size() << " bytes" << std::endl;

        // 🔹 Print content preview (safe for binary)
        std::cout << "[Content preview]" << std::endl;

        size_t previewSize = std::min((size_t)100, p.content.size());

        for (size_t j = 0; j < previewSize; ++j)
        {
            unsigned char c = p.content[j];

            if (std::isprint(c))
                std::cout << c;
            else
                std::cout << ".";
        }

        if (p.content.size() > previewSize)
            std::cout << "...";

        std::cout << std::endl;
    }

    std::cout << "\n===== END =====" << std::endl;
}


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
     _boundary.clear();
    _boundary.push_back('-');
    _boundary.push_back('-');
    _boundary.insert(_boundary.end(), boundaryStr.begin(), boundaryStr.end());
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
//interdire file_name = "../../etc/passwd"
bool Upload::isSafeFilename(const std::string& name)
{
    if (name.empty())
        return (false);
    if (name == "." || name == "..")
        return false;
    if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos
        ||  name.find("..") != std::string::npos )
        return false;
    if (name.find("..") != std::string::npos)
        return false;
    for (size_t i = 0; i < name.size(); ++i)
    {
        char c = name[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-'))
            return false;
    }
    return true;

}

//pour chaque parties on va, verifier si y a un fichier (content-disposition: filename=" " )
//si fichier il va falloir creer un fichier dans upload
//Attention verifier si filename is valid pas de ../
void Upload::ProcessParts()
{
    _uploadedFiles.clear();
    printParts(); // fonction de debug
    for (size_t i = 0; i < _parts.size(); ++i)
    {
        Part &p = _parts[i];
        std::string cd = p.headers["Content-Disposition"];
        std::string name;
        size_t name_pos = cd.find("name=\"");
        if (name_pos != std::string::npos)
        {
            name_pos += 6;
            size_t end = cd.find("\"", name_pos);
            name = cd.substr(name_pos, end - name_pos);
        }
        size_t file_pos = cd.find("filename=\"");
        
        //cas de formulaire remplis
        if (file_pos == std::string::npos)
        {
            std::string value(p.content.begin(), p.content.end());
            std::cout << "Form field: " << name << " = " << value << std::endl;
            continue;
        }
        //Cas de fichier a upload
        file_pos += 10;
        size_t fend = cd.find("\"", file_pos);
        std::string filename = cd.substr(file_pos, fend - file_pos);
        if (!isSafeFilename(filename))
        {
            std::cerr << "Unsafe filename rejected: " << filename << std::endl;
            continue;
        }

        //la je vais crer le fichier et remplir avec content
        std::string path = _uploadDir + "/" + filename;
        if (access(path.c_str(), F_OK) == 0)
        {
            std::cerr << "File already exists: " << filename << std::endl;
            continue;
        }
        std::ofstream ofs(path.c_str(), std::ios::binary);
        if (!ofs)
        {
            std::cerr << "Cannot write file: " << path << std::endl;
            continue;
        }

        ofs.write(
            reinterpret_cast<const char*>(p.content.data()),
            p.content.size()
        );
        ofs.close();
        _uploadedFiles.push_back(filename);

        //debug
        std::cout << "Uploaded file: " << filename << " (" << p.content.size() << " bytes)" << std::endl;
    }
}

void Upload::ParseBody(const Request &req)
{
    std::cout << "Request body size: " << req.getBodyBinary().size() << std::endl;
    const std::vector<unsigned char> &body = req.getBodyBinary();
    size_t pos = 0;
    
    while (1)
    {
        std::vector<unsigned char>::const_iterator it = std::search(body.begin() + pos, body.end(), _boundary.begin(), _boundary.end());
        if (it == body.end())
            break ;
        size_t boundary_pos = it - body.begin();

        if (boundary_pos + _boundary.size() + 2 <= body.size() && body[boundary_pos + _boundary.size()] == '-'
            && body[boundary_pos + _boundary.size() + 1] == '-')
            break;
        
        pos = boundary_pos + _boundary.size() + 2;
        static const unsigned char sep[] = {'\r','\n','\r','\n'};
        std::vector<unsigned char>::const_iterator header_end_it  = std::search(body.begin() + pos, body.end(), sep, sep + 4);
        if (header_end_it == body.end())
            break;
        std::string headersStr(body.begin() + pos, header_end_it);
        pos = (header_end_it - body.begin()) + 4;

        std::vector<unsigned char>::const_iterator content_end_it = std::search(body.begin() + pos, body.end(), _boundary.begin(), _boundary.end());
        std::vector<unsigned char> content(body.begin() + pos, content_end_it);

        Part p;
        p.content = content;
        p.headers = FillHeaders(headersStr);
        _parts.push_back(p);

        pos = content_end_it - body.begin();
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
    std::cout << "good dir" << std::endl;  
    if (!canWrite(_uploadDir))
        return Response::Error(403, "No write permission in upload folder");


    
    std::cout << "--------- Handling upload ------" << std::endl;
    // ----- FAKE BODY POUR TEST -----
    // Seulement pour tester le parsing multipart avec 2 parts
    // std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    // std::string bodyStr =
    //     "--" + boundary + "\r\n"
    //     "Content-Disposition: form-data; name=\"file3\"; filename=\"test3.txt\"\r\n"
    //     "Content-Type: text/plain\r\n"
    //     "\r\n"
    //     "Hello from file 3333333\n"
    //     "\r\n"
    //     "--ekip ekip" + boundary + "\r\n"
    //     "Content-Disposition: form-data; name=\"file4\"; filename=\"../test4.txt\"\r\n"
    //     "Content-Type: text/plain\r\n"
    //     "\r\n"
    //     "Hello from file 2\n"
    //     "--" + boundary + "--\r\n";

    // Copie de la requête pour injecter le body factice
    // Request fakeReq = req;
    // fakeReq.getBodyBinary() = std::vector<unsigned char>(bodyStr.begin(), bodyStr.end());
    // fakeReq.getBody() = bodyStr;
    // Convertir le boundary en vector<unsigned char>
    // _boundary = std::vector<unsigned char>(boundary.begin(), boundary.end());    
    // fakeReq.displayRequest();

    // ----SI ON FAIT AVEC LA VRAI REQUEST RECU------
    // chercher la boundaries dans le header
    std::string contentType = req.getHeader("Content-Type");
    unsigned int pos = contentType.find("boundary=");
    std::string boundaryStr = contentType.substr(pos + 9);
    std::cout << "BOUNDARY" << std::endl;
    std::cout << boundaryStr << std::endl;
    _boundary = std::vector<unsigned char>(boundaryStr.begin(), boundaryStr.end());      


    // Parser le body factice et découper en parts
    _parts.clear();
    req.displayRequest();
    // vrai request
    ParseBody(req);
    ProcessParts();

    Response res;
    return (res.buildUploadResponse(_uploadedFiles));
}
