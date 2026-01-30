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

struct LocationConfig
{
    std::string                 path;
    std::string                 root;
    std::string                 index;
    std::vector<std::string>    allowMethods;
    size_t                      _maxBodySize;

    bool hasRoot;
    bool hasIndex;
    bool autoindex;
    bool hasAutoindex;
    bool hasAllowMethods;
    bool hasMaxBodySize;

    LocationConfig()
        : hasRoot(false), hasIndex(false),
          autoindex(false), hasAutoindex(false),
          hasAllowMethods(false), hasMaxBodySize(false) {}
};

#endif
