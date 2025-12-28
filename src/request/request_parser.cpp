#include "../include/request/request.hpp"

request::request()
{
    
}

request::request(std::string)
{
    
}

void    request::parseRequest(char *request)
{
    std::ifstream request_stream(request);
    std::string line;

    if (!request)
        throw std::runtime_error("couldn't open the request");
    while (std::getline(request_stream, line))
    {
        parse_request_first_line(request_stream);
        parse_header(request_stream);
    }   
}

void    request::parse_request_first_line(std::ifstream &stream)
{
    std::string word;

    stream >> word;
    _method = word;
    stream >> word;
    _request_target = word;
    stream >> word;
    _protocol = word;
}

void request::parse_header(std::ifstream &stream)
{
    std::string key;
    std::string line;
    while (std::getline(stream, key, ':'))
    {

        while(std::getline(stream, line, '\n'))
        {
            _header[key] = line;
        }
    }
}

void request::display_request()
{
    std::map<std::string, std::string>::iterator it = _header.begin();
    std::cout << _method << " "
            <<  _request_target << " "
            << _protocol << std::endl;
    while (it != _header.end())
    {
        std::cout << it->first << " : " << it->second << std::endl;
        ++it;
    }
    // body
}