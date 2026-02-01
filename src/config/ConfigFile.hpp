#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

# include <iostream>
# include <fstream>
# include <string>
# include <cstring>
# include <cstdlib>
# include <arpa/inet.h>
# include <vector>
#include <unistd.h>
# include <sstream>
# include <map>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <LocationConfig.hpp>

class ServerConfig
{
    private :
        int                         _listenPort;
        std::string                 _listenHost;
        std::string                 _serverName;
        std::string                 _root;
        std::string                 _index;
        std::vector<LocationConfig> _locations;
        std::map<int, std::string>  _errorPages;

        bool                        _hasListen;
        bool                        _hasServerName;
        bool                        _hasRoot;
        bool                        _hasIndex;


    public:
        int createListener();
        ServerConfig()
            : _listenPort(0),
            _hasListen(false),
            _hasServerName(false),
            _hasRoot(false),
            _hasIndex(false)
        {std::cout << "Server Config constructor called" << std::endl;};
        ~ServerConfig() {std::cout << "Server Config destructor called" << std::endl;};

        // //GETTER
        // const int         getListenPort() const {return (_listenPort);};
        // const std::string& getListenHost() const {return (_listenHost);};
        // const std::string& getServerName() const {return (_serverName);};
        // const std::string& getRoot() const {return (_root);};
        // const std::string& getIndex() const {return (_index);};
        // const std::vector<LocationConfig>& getLocations() const {return (_locations);};
        // const std::map<int, std::string>& getErrorPages() const {return (_errorPages);};
        const bool&        getHasListen() const {return (_hasListen);};
        // const bool&        getHasServerName() const {return (_hasServerName);};
        // const bool&        getHasRoot() const {return (_hasRoot);};
        // const bool&        getHasIndex() const {return (_hasIndex);};

        // //SETTER
        // void setListenPort(int listenPort) {this->_listenPort = listenPort;};
        // void setListenHost(std::string listenHost) {this->_listenHost = listenHost;};
        // void setServerName(std::string serverName) {this->_serverName = serverName;};
        // void setRoot(std::string root) {this->_root = root;};
        // void setIndex(std::string index) {this->_index = index;};
        // void setLocations(std::vector<LocationConfig> locations) {this->_locations = locations;};
        // void setErrorPages(std::map<int, std::string> errorPages) {this->_errorPages = errorPages;};
        // void setHasListen(bool hasListen) {this->_hasListen = hasListen;};
        // void setHasServerName(bool hasServerName) {this->_hasServerName = hasServerName;};
        // void setHasRoot(bool hasRoot) {this->_hasRoot = hasRoot;};
        // void setHasIndex(bool hasIndex) {this->_hasIndex =  hasIndex;};


        bool isServerStart(const std::string &line);
        void parseServer(std::ifstream &file);

        //ServerConfig
        void parseListenDirective(const std::string &t);
        void parseServerNameDirective(const std::string &t);
        void parseRootDirective(const std::string &t);
        void parseIndexDirective(const std::string &t);
        void parseErrorPageDirective(const std::string &t);
        void parseServerLine(const std::string &t);

        void applyServersDefaults();
};

//utils
bool isValidIPv4(const std::string &ip);
std::string trim(const std::string &s);
std::string removeSemicolon(const std::string &s);

#endif
