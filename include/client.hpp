#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>

class Client 
{
    int fd;
    std::string requestBuffer;
    std::string responseBuffer;
    bool ReadyToWrite;
    Client(): ReadyToWrite(false) {}
};

#endif