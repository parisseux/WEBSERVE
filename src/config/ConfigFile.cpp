#include "../../include/config.hpp"

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

bool initServers(const std::string &configFile, std::vector<ServerConfig> &servers)
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
                servers.push_back(server);
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