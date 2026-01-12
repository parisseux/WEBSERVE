#include "../../include/config.hpp"

void parseListenDirective(ServerConfig &server, const std::string &t)
{
    if (server.hasListen)
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
    if (host != "localhost" && host != "*" && !isValidIPv4(host))
        throw std::runtime_error("Invalid listen IP address: " + host);
    if (host == "*")
        host = "0.0.0.0";
    server.listenHost = host;
    server.listenPort = port;
    server.hasListen = true;
}

void parseServerNameDirective(ServerConfig &server, const std::string &t)
{
    if (server.hasServerName)
        throw std::runtime_error("Duplicate 'server_name' directive");
    server.serverName = removeSemicolon(t.substr(12));
    if (server.serverName.empty())
        throw std::runtime_error("server_name is empty");
    server.hasServerName = true;
}

void parseRootDirective(ServerConfig &server, const std::string &t)
{
    if (server.hasRoot)
        throw std::runtime_error("Duplicate 'root' directive");
    server.root = removeSemicolon(t.substr(5));
    if (server.root.empty())
        throw std::runtime_error("root is empty");
    server.hasRoot = true;
}

void parseIndexDirective(ServerConfig &server, const std::string &t)
{
    if (server.hasIndex)
        throw std::runtime_error("Duplicate 'index' directive");
    server.index = removeSemicolon(t.substr(6));
    if (server.index.empty())
        throw std::runtime_error("index is empty");
    server.hasIndex = true;
}

void parseErrorPageDirective(ServerConfig &server, const std::string &t)
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
    if (server.errorPages.find(code) != server.errorPages.end())
        throw std::runtime_error("error_page: duplicate definition for code " + codeStr);
    server.errorPages[code] = uri;
}

void parseServerLine(ServerConfig &server, const std::string &t)
{
    if (t.empty())
        return;
    if (t.find("listen ") == 0)
        parseListenDirective(server, t);
    else if (t.find("server_name ") == 0)
        parseServerNameDirective(server, t);
    else if (t.find("root ") == 0)
        parseRootDirective(server, t);
    else if (t.find("index ") == 0)
        parseIndexDirective(server, t);
    else if (t.find("error_page ") == 0)
        parseErrorPageDirective(server, t);
    else
        throw std::runtime_error("Unknown directive inside server: " + t);
}