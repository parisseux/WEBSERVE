
#include "../../include/webserv.hpp"

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


// std::string&    Request::getMethod()
// {
//     return(_method);
// }

// std::string&    Request::getRequestTarget()
// {
//     return(_request_target);
// }

// std::string&    Request::getPath()
// {
//     return(_path);
// }

// std::string&    Request::getQuery()  
// {
//     return(_query);
// }

// std::string&    Request::getProtocol() 
// {
//     return(_protocol);
// }

// std::string&    Request::getBody()
// {
//     return(_body);
// }



