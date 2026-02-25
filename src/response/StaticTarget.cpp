#include "StaticTarget.hpp"
#include "../client/client.hpp"
#include "autoindex.hpp"

//std::ios::binary empeche transformation automatique et du coup permet de lire differents type de contenu
//et coherence entre les ios
bool StaticTarget::ReadFile(const std::string &path, std::string &content, Client *client)
{
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;
    if (client->getByteSentPos() > 0)
        file.seekg(client->getByteSentPos());
    char buf[100];
    file.read(buf, sizeof(buf));
    ssize_t byteRead = file.gcount();
    client->addByteSentPos(byteRead);
    if (file.eof()) // check si on a lu touts le contenu
    {
        client->setResponseComplete(true);
        file.close();
    }
    else // on a pas lu tout le content
    {
        client->setResponseComplete(false);
    }
    content.append(buf, byteRead);
    return true;
}

std::string StaticTarget::getContentType(const std::string& path)
{
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "application/octet-stream";
    std::string ext = path.substr(dot + 1);

    if (ext == "html" || ext == "htm")
        return "text/html; charset=utf-8";
    if (ext == "css")
        return "text/css; charset=utf-8";
    if (ext == "js")
        return "application/javascript; charset=utf-8";
    if (ext == "png")
        return "image/png";
    if (ext == "jpg" || ext == "jpeg")
        return "image/jpeg";
    if (ext == "gif")
        return "image/gif";
    if (ext == "svg")
        return "image/svg+xml";
    if (ext == "txt")
        return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

int StaticTarget::BuildStaticResponse(const Request& req, const ResolvedTarget& target, Client *client, Response &res)
{
    if (target.type == AUTO_INDEX_TARGET)
    {
        if (req.getMethod() != "GET")
            return (405);
        std::string html = GenerateAutoIndexHtml(target.path, req.getPath());
        if (client->getResponseClass().getResponseState() == FIRST_SENT)
        {
            res.setStatus(200);
            res.setHeader("Content-Type", "text/html; charset=utf-8");
            std::ostringstream len;
            len << html.size();
            res.setHeader("Content-Length", len.str());
        }

        res.setBody(html);
        client->setResponseComplete(true);
        return (200);
    }
    if (client->getResponseClass().getResponseState() == FIRST_SENT) // check si on a deja recuperer le headers
    {
        res.setStatus(200);
        res.setHeader("Content-Type", getContentType(target.path));
        std::ostringstream len;
        len << target.st.st_size;
        res.setHeader("Content-Length", len.str());
    }
    // ensuite lire le fichier jusqua un certain point
    // garder le point et lire depuis ce point pour la prochaine fois
    std::string content;
    if (!ReadFile(target.path, content, client))
        return 403 ;
    res.setBody(content);
    return (200);
}

bool StaticTarget::IsLocationPrefix(const std::string& reqPath, const std::string& locPath)
{
    if (locPath == "/") 
        return true;
    if (reqPath.compare(0, locPath.size(), locPath) != 0)
        return false;
    if (reqPath.size() == locPath.size()) 
        return true;
    return reqPath[locPath.size()] == '/';
}

bool StaticTarget::ContainsDotDotSegment(const std::string& rel)
{
    size_t i = 0;
    while (i < rel.size())
    {
        while (i < rel.size() && rel[i] == '/') i++;
        size_t j = i;
        while (j < rel.size() && rel[j] != '/') j++;
        if (j > i)
        {
            std::string seg = rel.substr(i, j - i);
            if (seg == "..") return true;
        }
        i = j;
    }
    return false;
}

std::string StaticTarget::GetEffectiveRoot(const ServerConfig &server, const LocationConfig &loc)
{
    if (loc.getHasRoot())
        return (loc.getRoot());
    else 
        return (server.getRoot());
}

//relativPath = req.path - matchloc.path
//cat.png = /image/cat.png - /images/
std::string StaticTarget::GetRelativPath(const std::string &reqPath, const std::string &locPath)
{
    std::string relativePath = reqPath;
    if (IsLocationPrefix(reqPath, locPath))
        relativePath.erase(0, locPath.size());
    if (!relativePath.empty() && relativePath[0] == '/')
        relativePath.erase(0, 1);
    return relativePath;
}

//./root/relativPath
std::string StaticTarget::JoinPath(const std::string &root, const std::string &relativPath)
{
    if (relativPath.empty())
        return root;
    std::string r = root;
    std::string p = relativPath;
    if (!root.empty() && root[root.size() - 1] == '/')
        r.erase(r.size() - 1);
    if (!p.empty() && p[0] == '/')
        p.erase(0, 1);
    return r + "/" + p;
}

ResolvedTarget StaticTarget::ResolveStaticTarget(const Request &req, const ServerConfig &server, const LocationConfig &loc)
{
    ResolvedTarget r;
    r.status = 200;
    r.reason = "OK";
    r.type = FILE_TARGET;
    std::string root = GetEffectiveRoot(server, loc);
    std::string rel  = GetRelativPath(req.getPath(), loc.getPath());
    if (ContainsDotDotSegment(rel))
    {
        r.status = 403;
        r.reason = "Forbidden";
        return r;
    }
    
    std::string path = JoinPath(root, rel);

    struct stat st;
    if (stat(path.c_str(), &st) != 0)
    {
        if (errno == ENOENT) 
        { 
            r.status = 404; 
            r.reason = "Not Found";
        }
        else 
        { 
            r.status = 403;
            r.reason = "Forbidden";
        }
        return r;
    }
    // Directory -> index.html
    if (S_ISDIR(st.st_mode))
    {
        std::string index = path;
        if (!index.empty() && index[index.size() - 1] != '/')
            index += '/';
        index += "index.html";

        struct stat stIndex;

        //cas ou on trouve index.html 
        if (stat(index.c_str(), &stIndex) == 0 && S_ISREG(stIndex.st_mode))
        {
            r.type = FILE_TARGET;
            r.path = index;
            r.st   = stIndex;
            return r;
        }
        //autoindex off
        if (!loc.getAutoIndex())
        {
            r.status = 403;
            r.reason = "Directory listing forbidden";
            return r;
        }
        //autoindex on
        r.path = path;
        r.st = st;
        r.type = AUTO_INDEX_TARGET;
        return r;        
    }
    //fichier normal, cad quon peut open()
    if (!S_ISREG(st.st_mode))
    {
        r.status = 403;
        r.reason = "Not a regular file";
        return r;
    }
    r.path = path;
    r.st = st;
    r.type = FILE_TARGET;
    return r;
}