
#include "LocationConfig.hpp"
#include "ConfigFile.hpp"

void LocationConfig::parseLocationRoot(const std::string &s)
{
    if (this->_hasRoot)
        throw std::runtime_error("Duplicate 'root' directive in location " + this->_path);
    this->_root = removeSemicolon(s.substr(5));
    if (this->_root.empty())
        throw std::runtime_error("Empty root in location " + this->_path);
    this->_hasRoot = true;
}

void LocationConfig::parseLocationIndex(const std::string &s)
{
    if (this->_hasIndex)
        throw std::runtime_error("Duplicate 'index' directive in location " + this->_path);
    this->_index = removeSemicolon(s.substr(6));
    if (this->_index.empty())
        throw std::runtime_error("Empty index in location " + this->_path);
    this->_hasIndex = true;
}

void LocationConfig::parseLocationAutoindex(const std::string &s)
{
    if (this->_hasAutoindex)
        throw std::runtime_error("Duplicate 'autoindex' in location " + this->_path);
    std::string val = removeSemicolon(s.substr(10));
    if (val == "on")
        this->_autoindex = true;
    else if (val == "off")
        this->_autoindex = false;
    else
        throw std::runtime_error("Invalid autoindex value in location " + this->_path);
    this->_hasAutoindex = true;
}

void LocationConfig::parseLocationMaxBodySize(const std::string &s)
{
    if (this->_hasMaxBodySize)
        throw std::runtime_error("Duplicate 'client_max_body_size' in location " + this->_path);
    std::string val = removeSemicolon(s.substr(20));
    if (val.empty())
        throw std::runtime_error("Empty client_max_body_size in location " + this->_path);
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
    this->_maxBodySize = strtoul(numStr.c_str(), NULL, 10) * multiplier;
    this->_maxBodySize = true;
}

void LocationConfig::parseLocationAllowMethods(const std::string &s)
{
    if (this->_hasAllowMethods)
        throw std::runtime_error("Duplicate 'allow_methods' in location " + this->_path);
    std::string methodsStr = removeSemicolon(s.substr(13));
    if (methodsStr.empty())
        throw std::runtime_error("Empty allow_methods in location " + this->_path);
    std::istringstream iss(methodsStr);
    std::string m;
    while (iss >> m)
        this->_allowMethods.push_back(m);
    if (this->_allowMethods.empty())
        throw std::runtime_error("No methods in allow_methods for location " + this->_path);
    this->_hasAllowMethods = true;
}

void LocationConfig::parseLocationHeader(const std::string &firstLine)
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
    this->_path = path;
}

void LocationConfig::parseLocationLine(const std::string &s)
{
    if (s.empty())
        return;
    if (s.find("root ") == 0)
        this->parseLocationRoot(s);
    else if (s.find("index ") == 0)
        this->parseLocationIndex(s);
    else if (s.find("autoindex ") == 0)
        this->parseLocationAutoindex(s);
    else if (s.find("client_max_body_size ") == 0)
        this->parseLocationMaxBodySize(s);
    else if (s.find("allow_methods ") == 0)
        this->parseLocationAllowMethods(s);
    else
        throw std::runtime_error("Unknown directive in location " + this->_path + ": " + s);
}


//metode de LocationConfig pour eviter les Getter, mas dcp on doit
//passer le serverconfig en parametre,
//Est ce qu on fait une class location qui herite de server pour avoir
//les getters?
void LocationConfig::parseLocationDirective(ServerConfig& server, std::ifstream &file, const std::string &firstLine)
{
    this->parseLocationHeader(firstLine);
    std::string line;
    while (std::getline(file, line))
    {
        std::string s = trim(line);
        if (s == "}")
            break;
        this->parseLocationLine(s);
    }
    if (!this->_hasRoot)
        this->_root = server.getRoot();
    server.getLocations().push_back(*this);
}


//metode de LocationConfig pour eviter les Getter, mas dcp on doit
//passer le serverconfig en parametre,
//Est ce qu on fait une class location qui herite de server pour avoir
//les getters?
void LocationConfig::applyLocationDefaults(ServerConfig& server)
{
    if (!this->_hasAutoindex)
        this->_autoindex = false;
    if (!this->_hasAllowMethods)
        this->_allowMethods.push_back("GET");
    if (!this->_hasIndex)
        this->_index = server.getIndex();
    if (!this->_hasRoot)
        this->_root = server.getRoot();
}
