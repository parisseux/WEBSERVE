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

void print_servers_attributes(std::vector<ServerConfig> &servers)
{
    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerConfig &server = servers[i];
        std::cout << "Parsed server: " << i + 1<< std::endl;
        std::cout << "  Host  = " << server.listenHost << std::endl;
        std::cout << "  Port  = " << server.listenPort << std::endl;
        std::cout << "  Name  = " << server.serverName << std::endl;
        std::cout << "  Root  = " << server.root << std::endl;
        std::cout << "  Index = " << server.index << std::endl;
        if (!server.errorPages.empty())
        {
            std::cout << "  Error Pages:" << std::endl;
            for (std::map<int, std::string>::const_iterator it = server.errorPages.begin();
                it != server.errorPages.end(); ++it)
                std::cout << "    " << it->first << " -> " << it->second << std::endl;
        }
        else
            std::cout << "  Error Pages: none" << std::endl;

        if (server.locations.empty())
            std::cout << "  No locations defined." << std::endl;

        for (size_t i = 0; i < server.locations.size(); i++)
        {
            const LocationConfig &loc = server.locations[i];
            std::cout << "\n  Location " << i << ":" << std::endl;
            std::cout << "    path      = " << loc.path << std::endl;
            std::cout << "    root      = " << loc.root << std::endl;
            std::cout << "    index     = " << loc.index << std::endl;
            std::cout << "    autoindex = " << (loc.autoindex ? "on" : "off") << std::endl;
        }
    }
}