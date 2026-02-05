#include "cgi.hpp"
#include "../socket/epoll.hpp"

extern char** environ;

std::string Cgi::GetEffectiveRoot(const ServerConfig &server, const LocationConfig &loc)
{
    if (loc.getHasRoot())
        return (loc.getRoot());
    else 
        return (server.getRoot());
}

//relativPath = req.path - matchloc.path
//cat.png = /image/cat.png - /images/
std::string Cgi::GetRelativPath(const std::string &reqPath, const std::string &locPath)
{
    std::string relativePath = reqPath;
    if (relativePath.find(locPath) == 0)
        relativePath.erase(0, locPath.size());
    if (!relativePath.empty() && relativePath[0] == '/')
        relativePath.erase(0, 1);
    return relativePath;
}

//./root/relativPath
std::string Cgi::JoinPath(const std::string &root, const std::string &relativPath)
{
    if (relativPath.empty())
        return root;
    std::string r = root;
    std::string p = relativPath;
    if (!root.empty() && root[root.size() - 1] == '/')
        r.erase(r.size() - 1);
    if (!p.empty() && p[0] == '/')
        p.erase(0, 1);
    return r + "/" + p;
}

bool isCgi(const Request &req, const ServerConfig &server, const LocationConfig &loc)
{
    Cgi cgi; 
    std::string root = cgi.GetEffectiveRoot(server, loc);
    std::string rel  = cgi.GetRelativPath(req.getPath(), loc.getPath());
    std::string path = cgi.JoinPath(root, rel);    
    size_t dot = path.rfind('.');
    std::string ext = path.substr(dot + 1);
    if (ext == "py")
        return (true);
    return (false);
}

void Cgi::readFd(int fd, std::string &content)
{
    char buff[100 + 1];
    int byteRead;
    while ((byteRead = read(fd, buff, 100)) > 0)
    {
        if (byteRead == 0)
            break;
        buff[byteRead] = '\0';
        std::string buffString(buff);
        content += buffString;
        buffString.clear();
    }
}

void Cgi::addCgiEnv(Request &req, std::string path, std::vector<std::string> &envCgiString)
{
    std::cout << "MakeCGI ENV" << std::endl;
    std::string ENV[5] = {
        "REQUEST_METHOD=", "CONTENT_LENGTH=",
        "CONTENT_TYPE=", "SCRIPT_NAME=", "SERVER_PROTOCOL=",
    };
    envCgiString.push_back(ENV[0] += req.getMethod());
    if (req.getMethod() == "POST" || req.getMethod() == "DELETE")
    {
        envCgiString.push_back(ENV[1] += headerValue("Content-Length", req));
        envCgiString.push_back(ENV[2] += headerValue("Content-Type", req));       
    }
    envCgiString.push_back(ENV[3] += path);
    envCgiString.push_back(ENV[4] += req.getProtocol());
}

void Cgi::MakeCgiEnv(Request &req)
{
    for (int i = 0; environ[i] != NULL; ++i)
    {
        std::string line(environ[i]);
        _envCgiString.push_back(line);
    }
    addCgiEnv(req, _path, _envCgiString);
    for (unsigned int i = 0; i < _envCgiString.size(); ++i)
        _envCgi.push_back((char *)_envCgiString[i].c_str());
    _envCgi.push_back(NULL);
}

Response Cgi::handleCgi(Request &req, const ServerConfig &server, const LocationConfig &loc, std::map<int, Cgi*> &_CgiMap, Client *client, Epoll epoll)
{
    std::string root = GetEffectiveRoot(server, loc);
    std::string rel  = GetRelativPath(req.getPath(), loc.getPath());
    _path = JoinPath(root, rel);
    // std::cout << root << std::endl;
    // std::cout << rel << std::endl;
    // std::cout << path << std::endl;
    Cgi cgi; 
    pid_t pid;
    int		fd[2];
    // int status = 0;
    const char *pythonPath = "/usr/bin/python3";
    char *args[] = {
        (char*)"python3", 
        (char*)_path.c_str(), 
        NULL 
    };
    if (pipe(fd) == -1)
        std::cout << "Pipe function error" << std::endl;
    pid = fork();
    Response res;
    MakeCgiEnv(req);
    // for (int i = 0; server.env[i] != NULL; ++i)
    // {
    //     std::string line(server.env[i]);
    //     _envCgiString.push_back(line);
    // }
    // cgi.addCgiEnv(req, _path, _envCgiString);
    // for (unsigned int i = 0; i < _envCgiString.size(); ++i)
    //     _envCgi.push_back((char *)_envCgiString[i].c_str());
    // _envCgi.push_back(NULL);
    switch (pid)
    {
        case -1:
            std::cout << "fork error" << std::endl;
            break ;
        case 0:
            std::cout << "Child Process" << std::endl;      
            dup2(fd[0], STDIN_FILENO);
            dup2(fd[1], STDOUT_FILENO);   
            close(fd[0]);
            close(fd[1]);                   
            execve(pythonPath, args, _envCgi.data());
            exit(EXIT_SUCCESS);
            break ;
        default:
            std::cout << "Parent Process" << std::endl;

            // close(fd[0]);
            client->setCgiFd(fd[0]);
            write(fd[1], req.getBody().c_str(), req.getBody().size());
            close(fd[1]);                      
            // std::string content;            
            // waitpid(pid, &status, 0);
            // // readFd(fd[0], content);
            // // close(fd[0]);                    
            // res.setStatus(200);
            // res.setHeader("Content-Type", "text/html");
            // int len = content.size();
            // std::stringstream ss;
            // ss << len;
            // res.setHeader("Content-Length", ss.str());
            // res.setBody(content);
            _CgiMap.insert(std::make_pair(fd[0], this));
            int flags = fcntl(fd[0], F_GETFL, 0);
            fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);
            epoll.setEvent(EPOLLIN);
            epoll.setEventFd(fd[0]);
            epoll_ctl(epoll.getEpFd(), EPOLL_CTL_ADD, fd[0], epoll.getEvent());
            std::cout << "DONE with CGI" << std::endl;           
            return res;        
    } 
    return res;
}