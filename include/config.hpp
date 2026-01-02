#ifndef CONFIG_HPP
#define CONFIG_HPP

# include <iostream>
# include <fstream>
# include <string>
# include <cstdlib>
# include <arpa/inet.h>
# include <vector>
# include <sstream>
# include <map>

struct LocationConfig
{
    std::string path;
    std::string root;
    std::string index;
    std::vector<std::string> allowMethods;

    bool hasRoot;
    bool hasIndex;
    bool autoindex;
    bool hasAutoindex;
    bool hasAllowMethods;

    LocationConfig()
        : hasRoot(false), hasIndex(false),
          autoindex(false), hasAutoindex(false),
          hasAllowMethods(false) {}
};
 
struct ServerConfig
{
    int listenPort;
    std::string listenHost;
    std::string serverName;
    std::string root;
    std::string index;
    std::vector<LocationConfig> locations;
    std::map<int, std::string> errorPages;

    bool hasListen;
    bool hasServerName;
    bool hasRoot;
    bool hasIndex;

    ServerConfig()
        : listenPort(0),
          hasListen(false),
          hasServerName(false),
          hasRoot(false),
          hasIndex(false)
    {}
};

//ConfigFile
bool initServers(const std::string &configFile, std::vector<ServerConfig> &servers);

//ServerConfig
void parseServerLine(ServerConfig &server, const std::string &t);

//LocationConfig
void parseLocationDirective(ServerConfig &server, std::ifstream &file, const std::string &firstLine);

//utils
bool isValidIPv4(const std::string &ip);
std::string trim(const std::string &s);
std::string removeSemicolon(const std::string &s);
bool isServerStart(const std::string &line);

#endif
