#include "../../include/parsing.hpp"

static ServerConfig parseServer(std::ifstream &file)
{
    ServerConfig server;
    std::string line;

    while (std::getline(file, line))
    {
        std::string t = trim(line);

        if (t == "}")
            break;
        if (t.empty())
            continue;
        if (t.rfind("location ", 0) == 0)
            parseLocationDirective(server, file, t);
        else
            parseServerLine(server, t);
    }
    return server;
}

bool initServer(const std::string &configFile)
{
    std::ifstream file(configFile.c_str());
    if (!file.is_open())
    {
        std::cerr << "Error: could not open config file\n";
        return false;
    }
    std::string line;
    try
    {
        while (std::getline(file, line))
        {
            if (isServerStart(line))
            {
                ServerConfig server = parseServer(file);

                if (!server.hasIndex || !server.hasListen
                    || !server.hasRoot || !server.hasServerName)
                {
                    std::cerr << "Error: missing informations in [configuration file]\n";
                    return false;
                }
                std::cout << "-----------DEBUG---------" << std::endl;
                std::cout << "Parsed server:" << std::endl;
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
                    {
                        std::cout << "    " << it->first << " -> " << it->second << std::endl;
                    }
                }
                else
                {
                    std::cout << "  Error Pages: none" << std::endl;
                }

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
    }
    catch (std::exception &e)
    {
        std::cerr << "Config parsing error: " << e.what() << std::endl;
        return false;
    }
    return true;
}