#include "../../include/config.hpp"

static void parseLocationRoot(LocationConfig &loc, const std::string &s)
{
    if (loc.hasRoot)
        throw std::runtime_error("Duplicate 'root' directive in location " + loc.path);
    loc.root = removeSemicolon(s.substr(5));
    if (loc.root.empty())
        throw std::runtime_error("Empty root in location " + loc.path);
    loc.hasRoot = true;
}

static void parseLocationIndex(LocationConfig &loc, const std::string &s)
{
    if (loc.hasIndex)
        throw std::runtime_error("Duplicate 'index' directive in location " + loc.path);
    loc.index = removeSemicolon(s.substr(6));
    if (loc.index.empty())
        throw std::runtime_error("Empty index in location " + loc.path);
    loc.hasIndex = true;
}

static void parseLocationAutoindex(LocationConfig &loc, const std::string &s)
{
    if (loc.hasAutoindex)
        throw std::runtime_error("Duplicate 'autoindex' in location " + loc.path);
    std::string val = removeSemicolon(s.substr(10));
    if (val == "on")
        loc.autoindex = true;
    else if (val == "off")
        loc.autoindex = false;
    else
        throw std::runtime_error("Invalid autoindex value in location " + loc.path);
    loc.hasAutoindex = true;
}

static void parseLocationAllowMethods(LocationConfig &loc, const std::string &s)
{
    if (loc.hasAllowMethods)
        throw std::runtime_error("Duplicate 'allow_methods' in location " + loc.path);
    std::string methodsStr = removeSemicolon(s.substr(13));
    if (methodsStr.empty())
        throw std::runtime_error("Empty allow_methods in location " + loc.path);
    std::istringstream iss(methodsStr);
    std::string m;
    while (iss >> m)
        loc.allowMethods.push_back(m);
    if (loc.allowMethods.empty())
        throw std::runtime_error("No methods in allow_methods for location " + loc.path);
    loc.hasAllowMethods = true;
}

static void parseLocationHeader(LocationConfig &loc, const std::string &firstLine)
{
    std::string t = trim(firstLine);
    std::string rest = trim(t.substr(8));
    if (rest.empty())
        throw std::runtime_error("location: missing path");
    size_t bracePos = rest.find('{');
    if (bracePos == std::string::npos)
        throw std::runtime_error("location: missing '{'");
    std::string path = trim(rest.substr(0, bracePos));
    if (path.empty())
        throw std::runtime_error("location: empty path");
    loc.path = path;
}

static void parseLocationLine(LocationConfig &loc, const std::string &s)
{
    if (s.empty())
        return;
    if (s.find("root ") == 0)
        parseLocationRoot(loc, s);
    else if (s.find("index ") == 0)
        parseLocationIndex(loc, s);
    else if (s.find("autoindex ") == 0)
        parseLocationAutoindex(loc, s);
    else if (s.find("allow_methods ") == 0)
        parseLocationAllowMethods(loc, s);
    else
        throw std::runtime_error("Unknown directive in location " + loc.path + ": " + s);
}

void parseLocationDirective(ServerConfig &server, std::ifstream &file, const std::string &firstLine)
{
    LocationConfig loc;
    parseLocationHeader(loc, firstLine);
    std::string line;
    while (std::getline(file, line))
    {
        std::string s = trim(line);
        if (s == "}")
            break;
        parseLocationLine(loc, s);
    }
    if (!loc.hasRoot)
        loc.root = server.root;
    server.locations.push_back(loc);
}