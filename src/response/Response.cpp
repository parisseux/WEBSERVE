#include "Response.hpp"

Response Response::buildUploadResponse(const std::vector<std::string>& files)
{
    Response res;

    res.setStatus(201);
    res.setHeader("Content-Type", "text/plain");

    std::ostringstream body;
    body << "Upload successful\n";

    for (size_t i = 0; i < files.size(); ++i)
        body << "- " << files[i] << "\n";

    std::string bodyStr = body.str();
    res.setBody(bodyStr);

    std::ostringstream len;
    len << bodyStr.size();
    res.setHeader("Content-Length", len.str());
    res.setHeader("Connection", "close");

    return res;
}

std::string Response::makeStatusLine(int code)
{
        if (code == 200)
            return "HTTP/1.0 200 OK";
        if (code == 400)
            return "HTTP/1.0 400 Bad Request";
        if (code == 403)
            return "HTTP/1.0 403 Forbidden";
        if (code == 404)
            return "HTTP/1.0 404 Not Found";
        if (code == 405)
            return "HTTP/1.0 405 Method Not Allowed";
        if (code == 501)
            return "HTTP/1.0 501 Not Implemented";
        return "HTTP/1.0 500 Internal Server Error";
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

void Response::displayResponse()
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

std::string Response::addBodyToResponseBuffer()
{
    std::string response;    
    if (_body.empty() == 0)
        response.append(_body);
    return (response);
}

//-----THEORY----------
//API --> facon standardisé de demander qlc à un programme 
// et d'obtenir une réponse
//en gros un programme peut faire des choses, mais personne ne peut pas lui parler
//une API définit comment on lui parle, définit ce quon peut lui demander, 
//définit ce quil réponse 
//Attention une API est destiné à un programme pas à un humain


//------THEORY-------
//lien youtube: https://www.youtube.com/watch?v=qmpUfWN7hh4
//HTTP status code (IMPORTANT)
//1XX --> informational 
//2XX --> succes 
    //200 succesful 
    //201 something created 
    // 204 something deleted (as asked)
//3XX -->  Redirection 
//4XX --> Client error (le server dit au client de vérifier sa request)
    //400 bad request
    //401 unauthorized 
    //403 Forbiden (privacy)
    //404 not found 
    //429 too many request --> asked the client to slow down
//5XX --> Servor error 
    //500 server error 
    //502 bad gateway (server overload, misconfiguration, ...)
    //503 service unvaliable (at the moment)