
#include "Request.hpp"
#include "../cgi/cgi.hpp"
#include "../client/client.hpp"
#include "../response/Response.hpp"
#include "../response/StaticTarget.hpp"
#include "../socket/epoll.hpp"
#include <iomanip>

Request::Request(std::string request)
{
    parseRequest(request.c_str());
}

std::string headerValue(std::string key, Request &req)
{
    std::map<std::string, std::string>::iterator it = req.getHeaders().begin();
    while(it != req.getHeaders().end())
    {
        if (it->first == key)
            return(it->second);
        it++;
    }
    return("");
}

int Request::ValidateRequest(const Request &req)
{
    if (req.getMethod().empty())
        return 400;
    if (req.getPath().empty())
        return 400;
    if (req.getPath()[0] != '/')
        return 400;
    if (req.getProtocol() != "HTTP/1.1") //est ce quon autorise d'autre protocol??
        return 400;
    if (req.getMethod() != "GET" && req.getMethod() != "POST" 
        && req.getMethod() != "DELETE" && req.getMethod() != "HEAD" )
        return 501;
    return 0;
}

int Request::MethodAllowed(const Request& req, const LocationConfig* loc)
{
    if (!loc) 
        return 0;
    std::string m = req.getMethod();
    if (!loc->getHasAllowMethods())
        return 0;
    for (std::vector<std::string>::const_iterator it = loc->getAllowMethods().begin();
         it != loc->getAllowMethods().end();
         ++it)
    {
        if (m == *it)
            return 0;
        if (m == "HEAD" && *it == "GET")
            return 0;
    }
    return 405;
}

bool Request::StartsWith(const std::string& s, const std::string& prefix)
{
    if (prefix.size() > s.size())
        return false;
    if (s.compare(0, prefix.size(), prefix) != 0)
        return false;
    return true;
}

const LocationConfig *Request::MatchLocation(const std::string &reqLoc, const std::vector<LocationConfig> &locations)
{
    const LocationConfig* bestLoc = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < locations.size(); ++i)
    {
        const LocationConfig& loc = locations[i];
        if (StartsWith(reqLoc, loc.getPath()) && loc.getPath().size() > bestLen)
        {
            bestLoc = &loc;
            bestLen = loc.getPath().size();
        }
    }
    return (bestLoc);
}

void Request::parseBody(Client *client)
{
    std::cout << "PARSE BODY " << std::endl;
    _body = client->getRequestBuffer();
    _bodyBinary.assign(client->getRequestBuffer().begin(), client->getRequestBuffer().end());
    // printBodyDebug(this->_bodyBinary);
    client->setRequestComplete(true);
}

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

    std::getline(stream, line);
    while (std::getline(stream, line))
    {
        found = line.find_first_of(':');
        if (found == std::string::npos)
            break ;
        key = line.substr(0, found);
        value = line.substr(found + 2, line.size());
        _header[key] = value;
    }
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

//function de debug
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


void printBodyDebug(const std::vector<unsigned char>& body)
{
    std::cout << "Body size = " << body.size() << " bytes\n";

    for (size_t i = 0; i < body.size(); i++)
    {
        std::cout  << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << (int)body[i] << " ";
    }
    std::cout << std::dec << std::endl;
}