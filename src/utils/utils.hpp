#ifndef UTILS_HPP
# define UTILS_HPP

# include "../src/config/ConfigFile.hpp"
#include "../config/config.hpp"
#include <iostream>

void writeInAscii(std::string string);

void print_servers_attributes(std::vector<ServerConfig> &servers);

#endif