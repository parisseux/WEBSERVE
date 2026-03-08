#include "Response.hpp"

// Response Response::buildUploadResponse(const std::vector<std::string>& files)
// {
//     Response res;

//     res.setStatus(201);
//     res.setHeader("Content-Type", "text/plain");

//     std::ostringstream body;
//     body << "Upload successful\n";

//     for (size_t i = 0; i < files.size(); ++i)
//         body << "- " << files[i] << "\n";

//     std::string bodyStr = body.str();
//     res.setBody(bodyStr);

//     std::ostringstream len;
//     len << bodyStr.size();
//     res.setHeader("Content-Length", len.str());
//     res.setHeader("Connection", "close");

//     return res;
// }

Response Response::buildDeleteResponse(int hasBeenDeleted)
{
    // le cas au status 200 est pour un delete reussi avec un body
    if (hasBeenDeleted == 0)
    {
        this->setStatus(200);
        std::string path = "/app/www/delete/index.html";
        std::string body;

        int fd = open(path.c_str(), O_RDONLY);
        if (fd < 0)
        {
            std::cout << "Open failed" << std::endl;
            this->setStatus(500);
            this->setHeader("Content-Length", "0");
            close(fd);
            return (*this);
        }
        else
        {
            ssize_t bytes;
            char buf[4096];
            while ((bytes = read(fd, buf, sizeof(buf))) > 0)
                body.append(buf, bytes);
            close(fd);
        }
        std::ostringstream len;
        len << body.size();
        this->setBody(body);
        this->setHeader("Content-Type", "text/html");        
        this->setHeader("Content-Length", len.str());      
    }
    // else if (hasBeenDeleted == 0)
    // {
    //     this->setStatus(204);
    //     this->setHeader("Content-Length", "0");
    // }
    else if (hasBeenDeleted == -1)
    {
        this->setStatus(404);
        this->setHeader("Content-Length", "0");
    }

    return (*this);
}

std::string Response::makeStatusLine(int code)
{
        if (code == 200)
            return "HTTP/1.0 200 OK";
        else if (code == 204)
            return ("HTTP/1.0 204 No Content");
        else if (code == 400)
            return "HTTP/1.0 400 Bad Request";
        else if (code == 403)
            return "HTTP/1.0 403 Forbidden";
        else if (code == 404)
            return "HTTP/1.0 404 Not Found";
        else if (code == 405)
            return "HTTP/1.0 405 Method Not Allowed";
        else if (code == 501)
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

// Response Response::Error(int code, const std::string &s)
// {
//     Response res;
//     res.setStatus(code);
//     res.setHeader("Content-Type", "text/html");
//     res.setBody("<html><body><h1>" + s + "</h1></body></html>");
//     return (res);
// }

void Response::displayResponse()
{
    std::map<std::string, std::string>::iterator it = _headers.begin();
    std::cout << "* SERVER JUST create A Response *" << std::endl;
    std::cout << "Status: "<< _status << "\nStatus line: "
            <<  _statusLine << std::endl;
    while (it != _headers.end())
    {
        std::cout << it->first << ": " << it->second << std::endl;
        ++it;
    }
    std::cout << "BODY:" << std::endl;
    if (_body.empty() == 0)
    {
        std::cout << std::endl;
        std::cout << _body;
    }
    std::cout << std::endl;
    std::cout << "* END OF RESPONSE *" << std::endl;    
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

ssize_t Response::getContentLength()
{
    std::map<std::string, std::string>::iterator it = this->getHeaders().begin();
    while(it != this->getHeaders().end())
    {
        if (it->first == "Content-Length")
        {
            ssize_t content_length;
            char *pEnd;
            content_length = std::strtoul(it->second.c_str(), &pEnd, 10);          
            return(content_length);
        }
        it++;
    }
    return(0);
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