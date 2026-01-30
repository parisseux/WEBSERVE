#include "../../include/webserv.hpp"

bool isCgi(const Request &req, const ServerConfig &server, const LocationConfig &loc)
{
    std::string root = getEffectiveRoot(server, loc);
    std::string rel  = getRelativPath(req.getPath(), loc.path);
    std::string path = joinPath(root, rel);    
    size_t dot = path.rfind('.');
    std::string ext = path.substr(dot + 1);
    if (ext == "py")
        return (true);
    return (false);
}

void readFd(int fd, std::string &content)
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

void makeCgiEnv(Request &req, std::string path, char **cgiEnv)
{
    std::cout << "MakeCGI ENV" << std::endl;
    std::string ENV[5] = {
        "REQUEST_METHOD=", "CONTENT_LENGTH=",
        "CONTENT_TYPE=", "SCRIPT_NAME=", "SERVER_PROTOCOL=",
    };
    ENV[0] += req.getMethod();
    if (req.getMethod() == "POST" || req.getMethod() == "DELETE")
    {
        ENV[1] += headerValue("Content-Length", req);
        ENV[2] += headerValue("Content-Type", req);
    }
    ENV[3] += path;
    ENV[4] += req.getProtocol();
    for (int i = 0; i < 5; ++i)
    {
        cgiEnv[i] = (char *)ENV[i].c_str();
    }
    cgiEnv[6] = NULL;
}

void makeCgi(Request &req, std::string path, std::vector<std::string> &envCgiString)
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
Response handleCgi(Request &req, const ServerConfig &server, const LocationConfig &loc)
{
    std::string root = getEffectiveRoot(server, loc);
    std::string rel  = getRelativPath(req.getPath(), loc.path);
    std::string path = joinPath(root, rel);
    std::cout << root << std::endl;
    std::cout << rel << std::endl;
    std::cout << path << std::endl;     
    pid_t pid;
    int		fd[2];
    int status = 0;
    const char *pythonPath = "/usr/bin/python3";
    char *args[] = {
        (char*)"python3", 
        (char*)path.c_str(), 
        NULL 
    };
    if (pipe(fd) == -1)
        std::cout << "Pipe function error" << std::endl;
    pid = fork();
    Response res;
    std::vector<std::string> envCgiString;
    for (int i = 0; server.env[i] != NULL; ++i)
    {
        std::string line(server.env[i]);
        envCgiString.push_back(line);
    }
    makeCgi(req, path, envCgiString);
    std::vector<char*> envCgi;
    for (unsigned int i = 0; i < envCgiString.size(); ++i)
        envCgi.push_back((char *)envCgiString[i].c_str());
    envCgi.push_back(NULL);
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
            execve(pythonPath, args, envCgi.data());
            exit(EXIT_SUCCESS);
            break ;
        default:
            std::cout << "Parent Process" << std::endl;

            close(fd[0]);
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
            std::cout << "DONE with CGI" << std::endl;           
            return res;        
    } 
    return res;
}