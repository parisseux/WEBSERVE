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

static void readFd(int fd, std::string &content)
{
    char buff[100];
    while (read(fd, buff, 100))
    {
        std::string buffString(buff);
        content += buffString;
        std::cout << "read" << std::endl;
    }
}


Response handleCgi(const Request &req, const ServerConfig &server, const LocationConfig &loc)
{
    std::string root = getEffectiveRoot(server, loc);
    std::string rel  = getRelativPath(req.getPath(), loc.path);
    std::string path = joinPath(root, rel);    
    pid_t pid;
    int		fd[2];
    int status = 0;
    extern char **environ;
    if (pipe(fd) == -1)
        std::cout << "Pipe function error" << std::endl;
    pid = fork();
    const char *pythonPath = "/usr/bin/python3";

    // 2. Préparation des arguments (argv)
    // argv[0] est par convention le nom du programme
    // argv[1] est le script à exécuter
    char *args[] = {
        (char*)"python3", 
        (char*)"/home/masase/Desktop/WEBSERVE/sites_exemple/hot_take/cgi-bin/hello.py", 
        NULL 
    };    
    switch (pid)
    {
        case -1:
            std::cout << "fork error" << std::endl;
        case 0:
            std::cout << "Child Process" << std::endl;         
            dup2(fd[1], STDOUT_FILENO);
            execve(pythonPath, args, NULL);
            close(fd[0]);
            close(fd[1]);   
        default:
            std::cout << "Parent Process" << std::endl;        
            waitpid(pid, &status, 0);
            std::string content;
            readFd(fd[0], content);
            close(fd[0]);
            close(fd[1]);               
            Response res;
            res.setStatus(200);
            res.setHeader("Content-Type", "text/plain; charset=utf-8");
            // std::ostringstream len;
            // len << target.st.st_size;
            // res.setHeader("Content-Length", len.str());
            res.setBody(content);
            res.displayResponse();
            return res;               
    }
         
}