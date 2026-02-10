
#include "Request.hpp"

Request::Request()
{
    
}

Request::~Request()
{
    
}

Request::Request(std::string request)
{
    parseRequest(request.c_str());
}

std::string headerValue(std::string key, Request &req)
{
    // std::cout << "header Value" << std::endl;    
    std::map<std::string, std::string>::iterator it = req.getHeaders().begin();
    while(it != req.getHeaders().end())
    {
        if (it->first == key)
        {
            // std::cout << "found an occurrence" << std::endl;
            return(it->second);
        }
        it++;
    }
    return("");
}

