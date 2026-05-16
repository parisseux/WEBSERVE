#ifndef CONTEXT_HPP
# define CONTEXT_HPP

#include "ConfigFile.hpp"
#include "LocationConfig.hpp"

struct Context
{
    const LocationConfig*    loc;
    const ServerConfig*      serv;
};

#endif