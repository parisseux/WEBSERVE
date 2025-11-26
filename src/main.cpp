#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include "../include/parsing.hpp"

int mess_error(std::string mess, int exit_code)
{
    std::cerr << "Error: " << mess << std::endl;
    return exit_code;
}
int main(int ac, char **av)
{
    if (ac != 2)
        return(mess_error("usage: ./webserv [configuration file]", 1));
    if (!initServer(av[1]))
        return(mess_error("server initialisation failed.", 1));
    std::cout << "Lets start webserv!!!!!" <<std::endl;
    return (0);
}