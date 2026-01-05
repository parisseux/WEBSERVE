#include "../include/utils/utils_1.hpp"

void writeInAscii(std::string string)
{
    std::string result;
    char c;
    for (size_t i = 0; i < string.length(); ++i)
    {
        c = string.at(i);
        std::cout << int(c) << ' ';
    }
}