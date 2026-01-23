
#include "../include/webserv.hpp"

static bool cgi(std::string &path, std::string &content)
{
    pid_t pid;
    std::stringstream ss(content); 

    pid = fork();
    switch (pid)
    {
        case -1:
            std::cout << "fork error" << std::endl;
        case 0:
            dup2(STDOUT_FILENO, ss);

    }

}