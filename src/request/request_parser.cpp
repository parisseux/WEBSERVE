#include "../include/request/request.hpp"
#include "../include/utils/utils_1.hpp"

request::request()
{
    
}

request::~request()
{
    
}

request::request(std::string request)
{
    parseRequest(request.c_str());
}

void    request::parseRequest(std::string request)
{
    std::stringstream request_stream(request);
    std::string line;

    if (!request_stream)
        throw std::runtime_error("couldn't open the request");
    parse_request_first_line(request_stream);
    parse_header(request_stream);
}

void    request::parse_request_first_line(std::stringstream &stream)
{
    std::string word;
    size_t found;    

    stream >> word;  
    _method = word;
    stream >> word;    
    _request_target = word;
    if ((found = word.find('?')) != std::string::npos)
    {
        _query = _request_target.substr(found + 1, _request_target.size());
        _request_target = _request_target.substr(0, found);
    }
    stream >> word;    
    _protocol = word;
}

void request::parse_header(std::stringstream &stream)
{
    std::string key;
    std::string value;
    std::string line;
    std::string trash;
    size_t found;

    std::getline(stream, line); // skip  \r \n line 
    while (std::getline(stream, line))
    {
        // std::cout << "line to parse: " << line << std::endl;
        // std::cout << "line to parse (ascii): ";
        // writeInAscii(line);
        // std::cout << std::endl;
        found = line.find_first_of(':');
        if (found == std::string::npos)
        {
            break ;
        }
        key = line.substr(0, found);
        value = line.substr(found + 2, line.size());
        _header[key] = value;
    }
}

void request::display_request()
{
    std::map<std::string, std::string>::iterator it = _header.begin();
    std::cout << "* SERVER JUST RECEIVED A REQUEST *" << std::endl;
    std::cout << _method << " "
            <<  _request_target << " "
            << _protocol << std::endl;
    while (it != _header.end())
    {
        std::cout << it->first << ": " << it->second << std::endl;
        ++it;
    }
    // body
}