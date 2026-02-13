#include "Request.hpp"

void    Request::parseRequest(std::string request)
{
    std::stringstream request_stream(request);
    std::string line;

    if (!request_stream)
        throw std::runtime_error("couldn't open the request");
    if (_method.empty())
    {
        parseRequestFirstLine(request_stream);
        parseHeader(request_stream);
    }
    //ATTEntion il faut pas parser le body ici ce nest pas une string
    //parseBody(request_stream);
}

void    Request::parseRequestFirstLine(std::stringstream &stream)
{
    std::string word;
    size_t found;

    stream >> word;  
    _method = word;
    stream >> word;    
    _requestTarget = word;
    if ((found = word.find('?')) != std::string::npos)
    {
        _query = _requestTarget.substr(found + 1);
        _requestTarget = _requestTarget.substr(0, found);
    }
    _path = _requestTarget;
    stream >> word;    
    _protocol = word;
}

void Request::parseHeader(std::stringstream &stream)
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

// void Request::parseBody(std::stringstream &stream)
// {
//     std::ostringstream oss;
//     oss << stream.rdbuf();
//     _body = oss.str();
// }

void Request::displayRequest() const
{
    std::map<std::string, std::string>::const_iterator it = _header.begin();
    std::cout << "* SERVER JUST RECEIVED A REQUEST *" << std::endl;
    std::cout << _method << " "
            <<  _requestTarget << " "
            << _protocol << std::endl;
    while (it != _header.end())
    {
        std::cout << it->first << ": " << it->second << std::endl;
        ++it;
    }
    if (_body.empty() == 0)
    {
        std::cout << "* BODY *" << std::endl;
        std::cout << _body;
    }
    std::cout << "* END OF REQUEST *"<< std::endl;
}

std::string Request::constructRequest()
{
    std::map<std::string, std::string>::iterator it = _header.begin();
    std::string request;
    request.append(_method);
    request.append(" ");
    request.append(_requestTarget);
    request.append(" ");
    request.append(_protocol);
    request.append("\r\n");           

    while (it != _header.end())
    {
        request.append(it->first);
        request.append(": ");
        request.append(it->second);
        request.append("\r\n");
        ++it;                            
    }
        request.append("\r\n");    
    if (_body.empty() == 0)
    {
        request.append(_body);
    }
    return (request);
}