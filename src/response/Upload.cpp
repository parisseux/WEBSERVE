#include "Upload.hpp"

#include <iostream>
#include <iomanip>
#include <cctype>

// fonction de debug pour voir les parts faire par CHATGPT
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
    std::string cl = req.getHeader("Content-Length");
    cl.erase(cl.find_last_not_of(" \r\n") + 1);
    if (cl.empty())
        return 400;
    char *endptr;
    long length = std::strtol(cl.c_str(), &endptr, 10);
    if (*endptr != '\0')
        return 400; 
    if (length <= 0)
        return 400;
    if (length > static_cast<long>(loc.getMaxBodySize()))
        return 413;
    return 200;
}

int Upload::CheckBodySize(const LocationConfig &loc, const Request &req)
{
    if (!loc.getHasMaxBodySize())
        return 200;
    if (req.getBody().size() > loc.getMaxBodySize())
        return 413;
    return (200);
}

std::string Upload::trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t'))
        start++;
    size_t end = s.size();
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n'))
        end--;
    return s.substr(start, end - start);
}

std::string Upload::extractBoundary(const std::string& contentType)
{
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        return "";
    std::string b = contentType.substr(pos + 9);
    size_t sc = b.find(';');
    if (sc != std::string::npos)
        b = b.substr(0, sc);
    b = trim(b);
    if (b.size() >= 2)
    {
        char first = b[0];
        char last = b[b.size() - 1];
        if ((first == '"' && last == '"') || (first == '\'' && last == '\''))
            b = b.substr(1, b.size() - 2);
    }
    return b;
}

std::map<std::string, std::string> Upload::FillHeaders(const std::string& headerStr)
{
    std::map<std::string, std::string> headers;
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
        std::string key = trim(line.substr(0, colon_pos));
        std::string value = trim(line.substr(colon_pos + 1));
        headers[key] = value;
    }
    return headers;
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

void Upload::ProcessParts()
{
    _uploadedFiles.clear();
    // printParts();
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
    _parts.clear();
    std::cout << "Request body size: " << req.getBodyBinary().size() << std::endl;
    const std::vector<unsigned char> &body = req.getBodyBinary();
    size_t pos = 0;
    static const unsigned char sep[] = {'\r','\n','\r','\n'};
    while (1)
    {
        std::vector<unsigned char>::const_iterator it =
            std::search(body.begin() + pos, body.end(), _delimiter.begin(), _delimiter.end());
        if (it == body.end())
            break;
        size_t boundary_pos = static_cast<size_t>(it - body.begin());
        pos = boundary_pos + _delimiter.size();
        if (pos + 1 < body.size() && body[pos] == '-' && body[pos + 1] == '-')
            break;
        if (pos + 1 < body.size() && body[pos] == '\r' && body[pos + 1] == '\n')
            pos += 2;
        std::vector<unsigned char>::const_iterator header_end_it =
            std::search(body.begin() + pos, body.end(), sep, sep + 4);
        if (header_end_it == body.end())
            break;
        std::string headersStr(body.begin() + pos, header_end_it);
        pos = static_cast<size_t>((header_end_it - body.begin()) + 4);
        std::vector<unsigned char> nextDelim;
        nextDelim.push_back('\r');
        nextDelim.push_back('\n');
        nextDelim.insert(nextDelim.end(), _delimiter.begin(), _delimiter.end());
        std::vector<unsigned char>::const_iterator content_end_it =
            std::search(body.begin() + pos, body.end(), nextDelim.begin(), nextDelim.end());
        if (content_end_it == body.end())
            break;
        std::vector<unsigned char> content(body.begin() + pos, content_end_it);
        Part p;
        p.content = content;
        p.headers = FillHeaders(headersStr);
        _parts.push_back(p);
        pos = static_cast<size_t>(content_end_it - body.begin());
    }
    std::cout << "Parsing of body finish" << std::endl;
}

int Upload::Handle(const LocationConfig &loc, const Request &req)
{
    if (!loc.getHasUploadPath())
        return (400);
    _uploadDir = loc.getUploadPath();
    int status = checkHeader(loc, req);
    if (status == 400)
        return (400);
    if (status == 413)
        return 413;
    if (!dirExists(_uploadDir))
        return 500;
    if (!canWrite(_uploadDir))
        return 403;

    // ----SI ON FAIT AVEC LA VRAI REQUEST RECU------
    // chercher la boundaries dans le header
    std::string contentType = req.getHeader("Content-Type");
    std::string boundary = extractBoundary(contentType);
    if (boundary.empty())
        return 400;
    std::string delimiterStr = "--" + boundary;
    _delimiter = std::vector<unsigned char>(delimiterStr.begin(), delimiterStr.end());   

    // Parser le body factice et découper en parts
    //req.displayRequest();
    // vrai request
    ParseBody(req);
    ProcessParts();

    // Response res_temp;
    // res = res_temp.buildUploadResponse(_uploadedFiles);
    return 200;
}
