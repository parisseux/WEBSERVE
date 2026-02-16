#include "utils.hpp"

void writeInAscii(std::string string)
{
    std::string result;
    char c;
    for (size_t i = 0; i < string.length(); ++i)
    {
        c = string.at(i);
        std::cout << int(c) << ' ';
    }
    std::cout << std::endl;
}

std::string trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

std::string removeSemicolon(const std::string &s)
{
    std::string out = trim(s);
    if (!out.empty() && out[out.size() - 1] == ';')
        out.erase(out.size() - 1);
    return trim(out);
}
