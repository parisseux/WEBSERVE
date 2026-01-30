#include "ConfigFile.hpp"

static void parseLocationMaxBodySize(LocationConfig &loc, const std::string &s)
{
    if (loc.getHasMaxBodySize())
        throw std::runtime_error("Duplicate 'client_max_body_size' in location " + loc.getPath());
    std::string val = removeSemicolon(s.substr(20));
    if (val.empty())
        throw std::runtime_error("Empty client_max_body_size in location " + loc.getPath());
    size_t multiplier = 1;
    char lastChar = val[val.size() - 1];
    if (lastChar == 'K' || lastChar == 'k')
        multiplier = 1024;
    else if (lastChar == 'M' || lastChar == 'm')
        multiplier = 1024*1024;
    else if (lastChar == 'G' || lastChar == 'g')
        multiplier = 1024*1024*1024;
    std::string numStr = val;
    if (!isdigit(lastChar))
        numStr = val.substr(0, val.size() - 1);
    loc.setMaxBodySize(strtoul(numStr.c_str(), NULL, 10) * multiplier);
    loc.setMaxBodySize(true);
}

static void parseLocationRoot(LocationConfig &loc, const std::string &s)
{
    if (loc.getHasRoot())
        throw std::runtime_error("Duplicate 'root' directive in location " + loc.getPath());
    loc.setRoot(removeSemicolon(s.substr(5)));
    if (loc.getRoot().empty())
        throw std::runtime_error("Empty root in location " + loc.getPath());
    loc.setHasRoot(true);
}

static void parseLocationIndex(LocationConfig &loc, const std::string &s)
{
    if (loc.getHasIndex())
        throw std::runtime_error("Duplicate 'index' directive in location " + loc.getPath());
    loc.setIndex(removeSemicolon(s.substr(6)));
    if (loc.getIndex().empty())
        throw std::runtime_error("Empty index in location " + loc.getPath());
    loc.setHasIndex(true);
}

static void parseLocationAutoindex(LocationConfig &loc, const std::string &s)
{
    if (loc.getHasAutoIndex())
        throw std::runtime_error("Duplicate 'autoindex' in location " + loc.getPath());
    std::string val = removeSemicolon(s.substr(10));
    if (val == "on")
        loc.setAutoIndex(true);
    else if (val == "off")
        loc.setAutoIndex(false);
    else
        throw std::runtime_error("Invalid autoindex value in location " + loc.getPath());
    loc.setHasAutoIndex(true);
}

static void parseLocationAllowMethods(LocationConfig &loc, const std::string &s)
{
    if (loc.getHasAllowMethods())
        throw std::runtime_error("Duplicate 'allow_methods' in location " + loc.getPath());
    std::string methodsStr = removeSemicolon(s.substr(13));
    if (methodsStr.empty())
        throw std::runtime_error("Empty allow_methods in location " + loc.getPath());
    std::istringstream iss(methodsStr);
    std::string m;
    while (iss >> m)
        loc.getAllowMethods().push_back(m);
    if (loc.getAllowMethods().empty())
        throw std::runtime_error("No methods in allow_methods for location " + loc.getPath());
    loc.setHasAllowMethods(true);
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
    loc.setPath(path);
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
    else if (s.find("client_max_body_size ") == 0)
        parseLocationMaxBodySize(loc, s);
    else if (s.find("allow_methods ") == 0)
        parseLocationAllowMethods(loc, s);
    else
        throw std::runtime_error("Unknown directive in location " + loc.getPath() + ": " + s);
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
    if (!loc.getHasRoot())
        loc.setRoot(server.getRoot());
    server.getLocations().push_back(loc);
}
