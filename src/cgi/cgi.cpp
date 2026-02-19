#include "cgi.hpp"
#include "../socket/epoll.hpp"
#include "../response/Response.hpp"
#include "../request/Request.hpp"
#include "wait.h"

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
    std::cout << "GetRelativPath: reqPath = " << reqPath << " locPath = " << locPath << std::endl;
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

void Cgi::handleCgi(Request &req, const ServerConfig &server, const LocationConfig &loc, Client *client, Epoll &epoll)
{
    std::string root = GetEffectiveRoot(server, loc);
    std::string rel  = GetRelativPath(req.getPath(), loc.getPath());
    std::cout << "ROOT: " << root << std::endl;
    std::cout << "REL: " << rel << std::endl;   

    _path = JoinPath(root, rel);
    std::cout << "PATH: " << _path << std::endl;
    req.displayRequest();
    pid_t pid;
    int		pipe_in[2];
    int     pipe_out[2];
    
    const char *pythonPath = "/usr/bin/python3";
    char *args[] = {
        (char*)"python3", 
        (char*)_path.c_str(), 
        NULL 
    };
    std::cout << "CGI PATH: " << _path << std::endl;
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
        std::cout << "Pipe function error" << std::endl;
    MakeCgiEnv(req);      
    pid = fork();
    switch (pid)
    {
        case -1:
            std::cout << "fork error" << std::endl;
            break ;
        case 0:
            // std::cout << "Child Process" << std::endl;      
            dup2(pipe_in[0], STDIN_FILENO);
            dup2(pipe_out[1], STDOUT_FILENO);   
            close(pipe_in[0]);
            close(pipe_in[1]);
            close(pipe_out[0]);
            close(pipe_out[1]);                  
            execve(pythonPath, args, _envCgi.data());
            exit(EXIT_SUCCESS);
            break ;
        default:
            // std::cout << "Parent Process" << std::endl;
            client->setCgiFd(pipe_out[0]);
            write(pipe_in[1], req.getBody().c_str(), req.getBody().size());  
            close(pipe_in[0]);
            close(pipe_in[1]);
            close(pipe_out[1]);                                                        
            // res.setStatus(200);
            // client->getResponseBuffer().push_front(res.AddToResponse());
            int flags = fcntl(pipe_out[0], F_GETFL, 0);
            fcntl(pipe_out[0], F_SETFL, flags | O_NONBLOCK);
            epoll.setEvent(EPOLLIN);
            epoll.setEventFd(pipe_out[0]);
            epoll_ctl(epoll.getEpFd(), EPOLL_CTL_ADD, pipe_out[0], epoll.getEvent());             
            std::cout << "DONE with CGI" << std::endl;                
    }
}