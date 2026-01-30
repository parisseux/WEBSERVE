#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

# include <iostream>
# include <fstream>
# include <string>
# include <cstring>
# include <cstdlib>
# include <arpa/inet.h>
# include <vector>
# include <sstream>
# include <map>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <LocationConfig.hpp>

class ServerConfig
{
    private :
        int                         listenPort;
        std::string                 listenHost;
        std::string                 serverName;
        std::string root;
        std::string index;
        std::vector<LocationConfig> locations;
        std::map<int, std::string> errorPages;

        bool hasListen;
        bool hasServerName;
        bool hasRoot;
        bool hasIndex;

    public:
        ServerConfig()
            : listenPort(0),
            hasListen(false),
            hasServerName(false),
            hasRoot(false),
            hasIndex(false)
        {std::cout << "Server Config constructor called" << std::endl;};
        ~ServerConfig() {std::cout << "Server Config destructor called" << std::endl;};
        int getListenPort() {return (listenPort);};
        std::string getListenHost() {return (listenHost);};
        std::string getServerName() {return (serverName);};
};

//ConfigFiles
void initServers(const std::string &configFile, std::vector<ServerConfig> &servers);

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
