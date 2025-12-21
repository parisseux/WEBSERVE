#include "../include/message.hpp"

void    request::parseRequest(char *request)
{
    std::ifstream request_stream(request);
    std::string line;

    if (!request)
        throw std::runtime_error("couldn't open the request");
    while (std::getline(request_stream, line))
    {
        
    }   
}