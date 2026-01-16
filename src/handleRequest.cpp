#include "../include/webserv.hpp"

static int validateRequest(Request &req)
{
    if (req.getMethod().empty())
        return 400;
    if (req.getPath().empty())
        return 400;
    if (req.getPath()[0] != '/')
        return 400;
    if (req.getProtocol() != "HTTP/1.1") //est ce quon autorise d'autre protocol??
        return 400;
    if (req.getMethod() != "GET" && req.getMethod() != "POST" 
        && req.getMethod() != "DELETE" && req.getMethod() != "HEAD" )
        return 501;
    return 0;
}

static int MethodAllowed(Request& req, const LocationConfig* loc)
{
    if (!loc) 
        return 0;
    std::string m = req.getMethod();
    if (!loc->hasAllowMethods)
        return 0;
    for (std::vector<std::string>::const_iterator it = loc->allowMethods.begin();
         it != loc->allowMethods.end();
         ++it)
    {
        if (m == *it)
            return 0;
        if (m == "HEAD" && *it == "GET")
            return 0;
    }
    return 405;
}

static bool startsWith(const std::string& s, const std::string& prefix)
{
    if (prefix.size() > s.size())
        return false;
    if (s.compare(0, prefix.size(), prefix) != 0)
        return false;
    return true;
}

const LocationConfig *MatchLocation(const std::string &reqLoc, const std::vector<LocationConfig> &locations)
{
    const LocationConfig* bestLoc = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < locations.size(); ++i)
    {
        const LocationConfig& loc = locations[i];
        if (startsWith(reqLoc, loc.path) && loc.path.size() > bestLen)
        {
            bestLoc = &loc;
            bestLen = loc.path.size();
        }
    }
    return (bestLoc);
}

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
    if (relativePath.find(locPath) == 0)
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

//std::ios::binary empeche transformation automatique et du coup permet de lire differents type de contenu
//et coherence entre les ios
static bool readFile(std::string &path, std::string &content)
{
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
    return true;
}

static std::string getContentType(const std::string& path)
{
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "application/octet-stream";
    std::string ext = path.substr(dot + 1);

    if (ext == "html" || ext == "htm") return "text/html; charset=utf-8";
    if (ext == "css") return "text/css; charset=utf-8";
    if (ext == "js") return "application/javascript; charset=utf-8";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "txt") return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

Response HandleRequest(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server)
{
    int status = validateRequest(req);
    if (status == 400)
        return Response::Error(400, "400 Bad Request");
    if (status == 501)
        return Response::Error(501, "501 Not Implemented");
    const LocationConfig *matchloc = MatchLocation(req.getPath(), locations);
    if (!matchloc)
        return Response::Error(500, "500 No location matched (unexpected)");
    status = MethodAllowed(req, matchloc);
    if (status == 405)
        return Response::Error(405, "405 Method Not Allowed");
    std::string root = getEffectiveRoot(server, *matchloc);
    std::string relativPath = getRelativPath(req.getPath(), matchloc->path);
    std::string finalPath = joinPath(root, relativPath);


    //stat va vérifier si le chemin existe, dis si fichier ou dossier, donne la taille, permet de savoir si accessible
    struct stat st;
    int ret = stat(finalPath.c_str(), &st);
    //renvoie 0 si trouver
    if (ret != 0)
    {
        if (errno == ENOENT)
            return Response::Error(404, "404 Not Found");
        else
            return Response::Error(403, "403 Forbidden");
    }
    //vérifie si c un fichier
    
    if (S_ISDIR(st.st_mode))
    {
        std::string indexPath = finalPath;
        if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
            indexPath += '/';
        indexPath += "index.html";

        struct stat stIndex;
        if (stat(indexPath.c_str(), &stIndex) != 0 || !S_ISREG(stIndex.st_mode))
            return Response::Error(403, "403 Directory access forbidden");
        finalPath = indexPath;
        st = stIndex;
    }
    if (S_ISREG(st.st_mode))
    {
        std::string FileContent;
        if (!readFile(finalPath, FileContent))
            return Response::Error(403, "403 Forbidden");
        std::cout << "FILE FOUND:\n" <<  finalPath << std::endl;
        Response res;
        res.setStatus(200);
        res.setHeader("Content-Type", getContentType(finalPath));
        std::ostringstream len;
        len << FileContent.size();
        res.setHeader("Content-Length", len.str());
        if (req.getMethod() != "HEAD")
            res.setBody(FileContent);
        return res;
    }  

    return Response::Error(403, "403 Forbidden");
}