#include "staticTarget.hpp"

std::string StaticTarget::getEffectiveRoot(const ServerConfig &server, const LocationConfig &loc)
{
    if (loc.hasRoot)
        return loc.root;
    return server.root;
}

std::string StaticTarget::getRelativPath(const std::string &reqPath, const std::string &locPath)
{
    std::string relative = reqPath;

    if (relative.find(locPath) == 0)
        relative.erase(0, locPath.size());

    if (!relative.empty() && relative[0] == '/')
        relative.erase(0, 1);

    return relative;
}

std::string StaticTarget::joinPath(const std::string &root, const std::string &relativPath)
{
    if (relativPath.empty())
        return root;

    std::string r = root;
    std::string p = relativPath;

    if (!r.empty() && r[r.size() - 1] == '/')
        r.erase(r.size() - 1);
    if (!p.empty() && p[0] == '/')
        p.erase(0, 1);

    return r + "/" + p;
}

ResolvedTarget StaticTarget::resolve(const Request &req, const ServerConfig &server, const LocationConfig &loc)
{
    ResolvedTarget r;
    r.status = 200;

    std::string root = getEffectiveRoot(server, loc);
    std::string rel  = getRelativPath(req.getPath(), loc.path);
    std::string path = joinPath(root, rel);

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

    if (S_ISDIR(st.st_mode))
    {
        std::string index = path;
        if (index[index.size() - 1] != '/')
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

bool StaticTarget::readFile(const std::string &path, std::string &content)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        return false;

    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
    return true;
}

std::string StaticTarget::getContentType(const std::string &path)
{
    size_t dot = path.rfind('.');
    if (dot == std::string::npos)
        return "application/octet-stream";

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

Response StaticTarget::buildResponse(const Request &req, const ResolvedTarget &target)
{
    if (target.status != 200)
        return Response::Error(target.status, target.reason);

    Response res;
    res.setStatus(200);
    res.setHeader("Content-Type", getContentType(target.path));

    std::ostringstream len;
    len << target.st.st_size;
    res.setHeader("Content-Length", len.str());

    if (req.getMethod() != "HEAD")
    {
        std::string content;
        if (!readFile(target.path, content))
            return Response::Error(403, "Forbidden");
        res.setBody(content);
    }
    return res;
}
