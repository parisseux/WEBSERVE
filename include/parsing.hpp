#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

struct ServerConfig
{
    int listenPort;
    std::string serverName;
    std::string root;
    std::string index;

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

bool isServerStart(const std::string &line);
ServerConfig parseServer(std::ifstream &file);
bool initServer(const std::string &configFile);

#endif
