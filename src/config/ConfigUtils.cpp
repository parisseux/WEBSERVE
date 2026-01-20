#include "../../include/webserv.hpp"

//sert à vérifier si une chaîne de caractères est une adresse IPv4 valide.
bool isValidIPv4(const std::string &ip)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 1;
}

std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

bool isServerStart(const std::string &line)
{
    std::string t = trim(line);
    if (t.rfind("server", 0) != 0)
        return false;
    std::string rest = trim(t.substr(6));
    return (rest == "{");
}

std::string removeSemicolon(const std::string &s)
{
    std::string out = trim(s);
    if (!out.empty() && out[out.size() - 1] == ';')
        out.erase(out.size() - 1);
    return trim(out);
}