#include "ConfigFile.hpp"

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

void initServers(const std::string &configFile, std::vector<ServerConfig> &servers)
{
    std::ifstream file(configFile.c_str());
    if (!file.is_open())
        throw std::runtime_error("could not open config file");
    servers.clear();
    std::string line;
    size_t serverCount = 0;
    std::cout << "Test" << std::endl;
    while (std::getline(file, line))
    {
        if (isServerStart(line))
            ++serverCount;
        try
        {
            ServerConfig server = parseServer(file);
            std::cout << "Test" << std::endl;
            if (!server.hasListen)
                throw std::runtime_error("missing listen directive");
            std::cout << "Test" << std::endl;
            applyServersDefaults(server);
            std::cout << "Test" << std::endl;
            servers.push_back(server);
        }
        catch (const std::exception& e) 
        {
            //throw std::runtime_error("config error in server block #" + std::to_string(serverCount) + ": " + e.what());
        }
    }
    if (servers.empty())
        throw std::runtime_error("no valid server block found in config");
}
