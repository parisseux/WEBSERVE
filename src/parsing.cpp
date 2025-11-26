#include "../include/parsing.hpp"

static std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

static bool isServerStart(const std::string &line)
{
    std::string t = trim(line);
    if (t.rfind("server", 0) != 0)
        return false;
    std::string rest = trim(t.substr(6));
    return (rest == "{");
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

        if (t.find("listen ") == 0)
        {
            if (server.hasListen)
                throw std::runtime_error("Duplicate 'listen' directive");
            std::string val = trim(t.substr(7));
            if (val.empty())
                throw std::runtime_error("Listen value is empty");
            server.listenPort = atoi(val.c_str());
            server.hasListen = true;
        }

        else if (t.find("server_name ") == 0)
        {
            if (server.hasServerName)
                throw std::runtime_error("Duplicate 'server_name' directive");
            server.serverName = trim(t.substr(12));
            if (server.serverName.empty())
                throw std::runtime_error("server_name is empty");
            server.hasServerName = true;
        }

        else if (t.find("root ") == 0)
        {
            if (server.hasRoot)
                throw std::runtime_error("Duplicate 'root' directive");
            server.root = trim(t.substr(5));
            if (server.root.empty())
                throw std::runtime_error("root is empty");
            server.hasRoot = true;
        }
  
        else if (t.find("index ") == 0)
        {
            if (server.hasIndex)
                throw std::runtime_error("Duplicate 'index' directive");
            server.index = trim(t.substr(6));
            if (server.index.empty())
                throw std::runtime_error("index is empty");
            server.hasIndex = true;
        }
        else if (!t.empty())
            throw std::runtime_error("Unknown directive inside server: " + t);
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
                if (server.hasIndex == false || server.hasListen == false
                    || server.hasRoot == false || server.hasServerName == false)
                {
                    std::cerr << "Error: missing informations in [configuration file]\n";
                    return false;
                } 
                std::cout << "Parsed server:" << std::endl;
                std::cout << "  Port  = " << server.listenPort << std::endl;
                std::cout << "  Name  = " << server.serverName << std::endl;
                std::cout << "  Root  = " << server.root << std::endl;
                std::cout << "  Index = " << server.index << std::endl;
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
