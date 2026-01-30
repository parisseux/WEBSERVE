#include "response.hpp"

std::string getEffectiveRoot(const ServerConfig &server, const LocationConfig &loc)
{
    if (loc.hasRoot)
        return (loc.root);
    else 
        return (server.root);
}

//relativPath = req.path - matchloc.path
//cat.png = /image/cat.png - /images/
std::string getRelativPath(const std::string &reqPath, const std::string &locPath)
{
    std::string relativePath = reqPath;
    (void)locPath;
    if (relativePath.find(locPath) == 0) // empeche d'acceder directement a une image par exemple
        relativePath.erase(0, locPath.size());
    if (!relativePath.empty() && relativePath[0] == '/')
        relativePath.erase(0, 1);
    return relativePath;
}

//./root/relativPath
std::string joinPath(const std::string &root, const std::string &relativPath)
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

ResolvedTarget resolveStaticTarget(const Request &req, const ServerConfig &server, const LocationConfig &loc)
{
    ResolvedTarget r;
    r.status = 200;

    std::string root = getEffectiveRoot(server, loc);
    std::string rel  = getRelativPath(req.getPath(), loc.path);
    std::string path = joinPath(root, rel);

    std::cout << root << std::endl;
    std::cout << rel << std::endl;
    std::cout << path << std::endl;
    std::cout << loc.path << std::endl;    
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