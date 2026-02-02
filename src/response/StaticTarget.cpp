#include "StaticTarget.hpp"

//std::ios::binary empeche transformation automatique et du coup permet de lire differents type de contenu
//et coherence entre les ios
bool StaticTarget::ReadFile(const std::string &path, std::string &content)
{
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
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

Response StaticTarget::BuildStaticResponse(const Request& req, const ResolvedTarget& target)
{
    if (target.status != 200)
    {
        if (target.status == 404)
            return Response::Error(404, "404 Not Found");
        if (target.status == 403)
            return Response::Error(403, "403 Forbidden");
        return Response::Error(target.status, "Error");
    }
    Response res;
    res.setStatus(200);
    res.setHeader("Content-Type", getContentType(target.path));
    std::ostringstream len;
    len << target.st.st_size;
    res.setHeader("Content-Length", len.str());
    if (req.getMethod() != "HEAD")
    {
        std::string content;
        if (!ReadFile(target.path, content))
            return Response::Error(403, "403 Forbidden");
        res.setBody(content);
    }
    return res;
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
    if (relativePath.find(locPath) == 0)
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

    std::string root = GetEffectiveRoot(server, loc);
    std::string rel  = GetRelativPath(req.getPath(), loc.getPath());
    std::string path = JoinPath(root, rel);

    struct stat st;
    if (stat(path.c_str(), &st) != 0)
    {
        if (errno == ENOENT) { r.status = 404; r.reason = "Not Found"; }
        else { r.status = 403; r.reason = "Forbidden"; }
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
        if (stat(index.c_str(), &stIndex) != 0 || !S_ISREG(stIndex.st_mode))
        {
            r.status = 403;
            r.reason = "Directory access forbidden";
            return r;
        }
        path = index;
        st = stIndex;
    }
    if (!S_ISREG(st.st_mode))
    {
        r.status = 403;
        r.reason = "Not a regular file";
        return r;
    }
    r.path = path;
    r.st = st;
    return r;
}