#include "../../include/config.hpp"

static void applyLocationDefaults(LocationConfig &loca, const ServerConfig &server)
{
    if (!loca.hasAutoindex)
        loca.autoindex = false;
    if (!loca.hasAllowMethods)
        loca.allowMethods.push_back("GET");
    if (!loca.hasIndex)
        loca.index = server.index;
    if (!loca.hasRoot)
        loca.root = server.root;
}

static void applyServersDefaults(ServerConfig &server)
{
    if (!server.hasRoot)
        server.root = "./www";
    if (!server.hasIndex)
        server.index = "index.html";
    if (!server.hasServerName)
        server.serverName = "";
    for (size_t i = 0; i < server.locations.size(); ++i)
        applyLocationDefaults(server.locations[i], server);
}

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
                if (!server.hasListen)
                {
                    std::cerr << "Error: missing listen port in [configuration file]\n";
                    return false;
                }
                applyServersDefaults(server);
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