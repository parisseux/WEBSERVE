#include "response.hpp"

std::string Response::makeStatusLine(int code)
{
        if (code == 200) return "HTTP/1.1 200 OK";
        if (code == 400) return "HTTP/1.1 400 Bad Request";
        if (code == 403) return "HTTP/1.1 403 Forbidden";
        if (code == 404) return "HTTP/1.1 404 Not Found";
        if (code == 405) return "HTTP/1.1 405 Method Not Allowed";
        if (code == 501) return "HTTP/1.1 501 Not Implemented";
        return "HTTP/1.1 500 Internal Server Error";
}

void Response::setStatus(int code) 
{ 
    _status = code;
    _statusLine = makeStatusLine(code);
}

void Response::setHeader(const std::string& k, const std::string& v)
{
    _headers[k] = v;
}
void Response::setBody(const std::string& b)
{
    _body = b;
}

Response Response::Error(int code, const std::string &s)
{
    Response res;
    res.setStatus(code);
    res.setHeader("Content-Type", "text/html");
    res.setBody("<html><body><h1>" + s + "</h1></body></html>");
    return (res);
}

void Response::display_response()
{
    std::map<std::string, std::string>::iterator it = _headers.begin();
    std::cout << "* SERVER JUST create A Response *" << std::endl;
    std::cout << "Status: "<< _status << "\nStatus line: "
            <<  _statusLine << "\nBody: " << _body << std::endl;
    while (it != _headers.end())
    {
        std::cout << it->first << ": " << it->second << std::endl;
        ++it;
    }
    if (_body.empty() == 0)
    {
        std::cout << std::endl;
        std::cout << _body;
    }
    std::cout << std::endl;
}

std::string Response::constructResponse()
{
    std::map<std::string, std::string>::iterator it = _headers.begin();
    std::string response;
    response.append(_statusLine);
    response.append("\r\n");
    while (it != _headers.end())
    {
        response.append(it->first);
        response.append(": ");
        response.append(it->second);
        response.append("\r\n");
        ++it;                            
    }
    response.append("\r\n"); 
    if (_body.empty() == 0)
    {
        response.append(_body);
    }
    return (response);
}

