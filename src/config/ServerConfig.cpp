#include "ConfigFile.hpp"
#include "../utils/utils.hpp"

void ServerConfig::parseListenDirective(const std::string &t)
{
    if (this->_hasListen)
        throw std::runtime_error("Duplicate 'listen' directive");
    std::string val = trim(t.substr(7));
    if (val.empty())
        throw std::runtime_error("Listen value is empty");
    std::string host;
    std::string portStr;
    size_t colon = val.find(':');
    if (colon != std::string::npos)
    {
        host = trim(val.substr(0, colon));
        portStr = trim(val.substr(colon + 1));
    }
    else
    {
        host = "0.0.0.0";
        portStr = val;
    }
    if (portStr.empty())
        throw std::runtime_error("Listen port missing");
    int port = atoi(portStr.c_str());
    if (port <= 0 || port > 65535)
        throw std::runtime_error("Invalid listen port");
    if (host != "localhost" && host != "*" && !this->isValidIPv4(host))
        throw std::runtime_error("Invalid listen IP address: " + host);
    if (host == "*")
        host = "0.0.0.0";
    this->_listenHost = host;
    this->_listenPort = port;
    this->_hasListen = true;
}

void ServerConfig::parseServerNameDirective(const std::string &t)
{
    if (this->_hasServerName)
        throw std::runtime_error("Duplicate 'server_name' directive");
    this->_serverName = removeSemicolon(t.substr(12));
    if (this->_serverName.empty())
        throw std::runtime_error("server_name is empty");
    this->_hasServerName = true;
}

void ServerConfig::parseRootDirective(const std::string &t)
{
    if (this->_hasRoot)
        throw std::runtime_error("Duplicate 'root' directive");
    this->_root = removeSemicolon(t.substr(5));
    if (this->_root.empty())
        throw std::runtime_error("root is empty");
    this->_hasRoot = true;
}

void ServerConfig::parseIndexDirective(const std::string &t)
{
    if (this->_hasIndex)
        throw std::runtime_error("Duplicate 'index' directive");
    this->_index = removeSemicolon(t.substr(6));
    if (this->_index.empty())
        throw std::runtime_error("index is empty");
    this->_hasIndex = true;
}

void ServerConfig::parseErrorPageDirective(const std::string &t)
{
    std::string rest = removeSemicolon(t.substr(10));
    if (rest.empty())
        throw std::runtime_error("error_page: missing arguments");
    std::istringstream iss(rest);
    std::string codeStr;
    std::string uri;
    if (!(iss >> codeStr))
        throw std::runtime_error("error_page: missing status code");
    if (!(iss >> uri))
        throw std::runtime_error("error_page: missing URI");
    uri = removeSemicolon(uri);
    int code = atoi(codeStr.c_str());
    if (code < 100 || code > 599)
        throw std::runtime_error("error_page: invalid status code");
    if (uri.empty())
        throw std::runtime_error("error_page: empty URI");
    if (this->_errorPages.find(code) != this->_errorPages.end())
        throw std::runtime_error("error_page: duplicate definition for code " + codeStr);
    this->_errorPages[code] = uri;
}

void ServerConfig::parseServerLine(const std::string &t)
{
    if (t.empty())
        return;
    if (t.find("listen ") == 0)
        this->parseListenDirective(t);
    else if (t.find("server_name ") == 0)
        this->parseServerNameDirective(t);
    else if (t.find("root ") == 0)
        this->parseRootDirective(t);
    else if (t.find("index ") == 0)
        this->parseIndexDirective(t);
    else if (t.find("error_page ") == 0)
        this->parseErrorPageDirective(t);
    else
        throw std::runtime_error("Unknown directive inside server: " + t);
}
