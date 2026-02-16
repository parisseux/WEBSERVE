#ifndef UTILS_HPP
# define UTILS_HPP

# include "../src/config/ConfigFile.hpp"
#include <iostream>

void        writeInAscii(std::string string);
std::string trim(const std::string& s);
std::string removeSemicolon(const std::string &s);

#endif
