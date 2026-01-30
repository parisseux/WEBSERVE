#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

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

class LocationConfig
{
  private:
    std::string                 _path;
    std::string                 _root;
    std::string                 _index;
    std::vector<std::string>    _allowMethods;
    size_t                      _maxBodySize;

    bool _hasRoot;
    bool _hasIndex;
    bool _autoindex;
    bool _hasAutoindex;
    bool _hasAllowMethods;
    bool _hasMaxBodySize;

  public:
    LocationConfig()
        : _hasRoot(false), _hasIndex(false),
          _autoindex(false), _hasAutoindex(false),
          _hasAllowMethods(false), _hasMaxBodySize(false)
        {std::cout << "Location Config constructor called" << std::endl;};
    ~LocationConfig() {std::cout << "Location Config destructor called" << std::endl;};

    //GETTERS
    const std::string&               getPath() const {return (_path);};
    const std::string&               getRoot() const {return (_root);};
    const std::string&               getIndex() const {return (_index);};
    const std::vector<std::string>&  getAllowMethods() const {return (_allowMethods);};
    const size_t&                    getMaxBodySize() const {return (_maxBodySize);};
    const bool&                      getHasRoot() const {return (_hasRoot);};
    const bool&                      getHasIndex() const {return (_hasIndex);};
    const bool&                      getAutoIndex() const {return (_autoindex);};
    const bool&                      getHasAutoIndex() const {return (_hasAutoindex);};
    const bool&                      getHasAllowMethods() const {return (_hasAllowMethods);};
    const bool&                      getHasMaxBodySize() const {return (_hasMaxBodySize);};

    //SETTERS
    void                       setPath(std::string path) {this->_path =  path;};
    void                       setRoot(std::string root) {this->_root =  root;};
    void                       setIndex(std::string index) {this->_index = index;};
    void                       setAllowMethods( std::vector<std::string> allowMethods) {this->_allowMethods = allowMethods;};
    void                       setMaxBodySize(size_t maxBodySize) {this->_maxBodySize = maxBodySize;};
    void                       setHasRoot(const bool hasRoot) {this->_hasRoot = hasRoot;};
    void                       setHasIndex(const bool hasIndex) {this->_hasIndex = hasIndex;};
    void                       setAutoIndex(const bool autoindex) {this->_autoindex = autoindex;};
    void                       setHasAutoIndex(const bool hasAutoindex) {this->_hasAutoindex = hasAutoindex;};
    void                       setHasAllowMethods(const bool hasAllowMethods) {this->_hasAllowMethods = hasAllowMethods;};
    void                       setHasMaxBodySize(const bool hasMaxBodySize) {this->_hasMaxBodySize = hasMaxBodySize;};
};

#endif
