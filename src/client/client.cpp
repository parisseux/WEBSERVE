#include "client.hpp"
#include "../response/Response.hpp"
#include "../cgi/cgi.hpp"
#include "../response/StaticTarget.hpp"

bool Client::isUpload()
{
    std::string type;
    type = headerValue("Content-Type", this->getRequestClass());
    if (type.find("multipart/form-data")!=std::string::npos)
        return true;
    return false;
}

unsigned int Client::getContentLength()
{
    std::string length;
    unsigned int content_length;
    char *pEnd;
    length = headerValue("Content-Length", this->getRequestClass());
    content_length = std::strtoul(length.c_str(), &pEnd, 10);
	if (*pEnd != '\0' && *pEnd != '\r')
	{    
		std::cout << "Error on content lenght ending char" << std::endl;              
	}    
    return (content_length);
}

int Client::getResponseBufferLength()
{
    std::deque<std::string>::iterator it;
    int length = 0;
    for (it = _responseBuffer.begin(); it != _responseBuffer.end(); ++it)
    {
        length += it->size();
    }
    return (length);
}

void Client::clearRequest()
{
    getRequestClass().getMethod().clear();
    getRequestClass().getRequestTarget().clear();
    getRequestClass().getPath().clear();
    getRequestClass().getQuery().clear();
    getRequestClass().getProtocol().clear();                     
    getRequestClass().getBody().clear();
    getRequestClass().getHeaders().clear();
}

void Client::clearResponse()
{
    getResponseClass().setStatus(200);
    getResponseClass().getHeaders().clear();
    getResponseClass().setBody("");
    getResponseClass().setResponseState(FIRST_READ);        
}

void Client::clearClient()
{
    setRequestComplete(false);
    clearRequest();
    getRequestBuffer().clear();
    clearResponse();							
    getResponseBuffer().clear();
    setClientState(WAITING);
    setResponseComplete(false);
    setByteReadPos(0);
    setByteSent(0); 
    close(this->getFd());    
}

void         Client::Handle(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server, Client *client, Epoll &epoll)
{
    int status = req.ValidateRequest(req);
    if (status != 200)
    {
        if (status == 400)
            sendError(400, "Bad Request", server);     
        else if (status == 501)
            sendError(501, "Not Implemented", server);
        return ; 
    }
    const LocationConfig *loc = req.MatchLocation(req.getPath(), locations);
    if (!loc)
    {
        sendError(500, "No location matched (unexpected)", server);
        return ;          
    }
    status = req.MethodAllowed(req, loc);
    if (status == 405)
    {
        sendError(405, "Method Not Allowed", server);
        return ;
    }
    if (loc->getHasRedirect())
    {
        std::cout << "Lets handle redirect HTTP" << std::endl;
        sendRedirect(loc->getRedirect());
        return ;
    }
    if (isCgi(req, server, *loc))
    {
        Cgi cgi;
        cgi.handleCgi(req, server, client, epoll);
        client->setClientState(GENERATING_CGI);
        return ;
    }
    if (req.getMethod() == "POST")
    {
        Upload up;
        status = up.CheckBodySize(*loc, req);
        if (status != 200)
        {
            sendError(413, "Payload Too Large", server);
            return ;         
        }
        if (!req.hasHeader("Content-Type") || !req.hasHeader("Content-Length"))
        {
            sendError(400, "Bad Request", server);        
            return ;
        }
        if (req.getHeader("Content-Type").rfind("multipart/form-data", 0) == 0 && req.getPath() == "/upload")
        {
            // Response uploadRes;
            // int upStatus = up.Handle(*loc, req, uploadRes);
            int upStatus = up.Handle(*loc, req);
            if (upStatus != 200)
            {
                if (upStatus == 413)
                    sendError(413, "Payload Too Large", server);
                else if (upStatus == 403)
                    sendError(403, "Forbidden", server);
                else if (upStatus == 400)
                    sendError(400, "Bad Request", server);
                else
                    sendError(500, "Internal Server Error", server);
                return;
            }
            sendUpload();
            return;
        }
    }
    StaticTarget st;
    ResolvedTarget target = st.ResolveStaticTarget(req, server, *loc);
    if (target.status != 200)
    {
        sendError(target.status, target.reason, server);
        return;
    }
    int stStatus = st.BuildStaticResponse(req, target, client, _response);
    if (stStatus != 200)
    {
        if (stStatus == 405)
            sendError(405, "Method Not Allowed", server);
        else
            sendError(403, "Forbidden", server);
        return;
    }
    if(_response.getResponseState() == FIRST_READ)
    {
        client->getResponseBuffer().push_front(_response.constructResponse().data());
        _response.setResponseState(NEXT_READ);
    }
    else
    {      
        client->getResponseBuffer().push_front(_response.getBody());
    }
    _response.getBody().clear();
}

void Client::sendError(int code, const std::string& reason, const ServerConfig& server)
{
    std::string path = server.getErrorPage(code);
    std::string body;
    std::string finalPath;
    if (path.empty())
        finalPath = "/app/www/errors/default.html";
    else
    {
        if (!path.empty() && path[0] == '/')
            finalPath = server.getRoot() + path;
        else
            finalPath = path;
    }
    int fd = open(finalPath.c_str(), O_RDONLY);
    if (fd >= 0)
    {
        char buffer[4096];
        ssize_t bytes;
        while ((bytes = read(fd, buffer, sizeof(buffer))) > 0)
            body.append(buffer, bytes);
        close(fd);
    }
    else 
            std::cout << "problème lorsqu'on essaie d'ouvrir l'error page" << std::endl;
    //AU CAS OU PROBLeME DOUVERTURE DE FICHIER jpense c bien quon garde 
    if (body.empty())
    {
        std::ostringstream ss;
        ss << "<html><body><h1>"
           << code << " " << reason
           << "</h1></body></html>";
        body = ss.str();
    }
    Response res;
    std::cout << "Code:  " << code << std::endl;
    res.setStatus(code);
    res.setBody(body);
    res.setHeader("Content-Type", "text/html");
    std::ostringstream len;
    len << body.size();
    res.setHeader("Content-Length", len.str());
    getResponseBuffer().push_front(res.constructResponse());
    setResponseComplete(true);
}

void Client::sendUpload()
{
    std::string finalPath = "/app/www/siteUpload/index.html";
    std::string body;

    int fd = open(finalPath.c_str(), O_RDONLY);
    if (fd >= 0)
    {
        char buffer[4096];
        ssize_t bytes;
        while ((bytes = read(fd, buffer, sizeof(buffer))) > 0)
            body.append(buffer, bytes);
        close(fd);
    }
    else 
            std::cout << "problème lorsqu'on essaie d'ouvrir l'upload page" << std::endl;
    if (body.empty())
    {
        std::ostringstream ss;
        ss << "<html><body><h1>"
           << 200 << " " << "Upload reussi mais récupération de fichier n'a pas fonctionné"
           << "</h1></body></html>";
        body = ss.str();
    }
    Response res;
    res.setStatus(200);
    res.setBody(body);
    res.setHeader("Content-Type", "text/html");
    std::ostringstream len;
    len << body.size();
    res.setHeader("Content-Length", len.str());
    getResponseBuffer().push_front(res.constructResponse());
    setResponseComplete(true);
}

void Client::sendRedirect(const std::string &redir)
{
    std::stringstream html;

html <<
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"  <meta charset=\"UTF-8\">\n"
"  <title>Redirect HTTP...</title>\n"
"  <style>\n"
"    body{margin:0;padding:40px;background:#fdeef3;font-family:Segoe UI,Tahoma,sans-serif;color:#5e4a54;}\n"
"    .container{max-width:700px;margin:auto;background:#fff7fa;padding:35px 45px;border-radius:22px;"
"box-shadow:0 10px 35px rgba(214,164,181,.25);text-align:center;}\n"
"    h1{margin-top:0;font-weight:500;font-size:24px;color:#c26d8d;}\n"
"    p{font-size:15px;color:#a05574;margin:20px 0;}\n"
"    a{text-decoration:none;color:#ffffff;background:#c26d8d;padding:10px 18px;"
"border-radius:14px;display:inline-block;transition:all .25s ease;font-size:14px;}\n"
"    a:hover{background:#a05574;}\n"
"    footer{margin-top:30px;font-size:12px;color:#c9a5b5;}\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"<div class=\"container\">\n"
"  <h1>🌸 Page moved</h1>\n"
"<p>This page has moved to: " << redir << "</p>\n"
"<a href=\"" << redir << "\">" << redir << "</a>\n"
"</div>\n"
"</body>\n"
"</html>\n";
    Response res;
    res.setStatus(301);
    std::string body = html.str();
    res.setHeader("Location", redir);
    std::ostringstream len;
    len << body.size();
    res.setHeader("Content-Length", len.str());
    res.setBody(body);
    getResponseBuffer().push_front(res.constructResponse());
    setResponseComplete(true);
}


