#include "client.hpp"
#include "../response/Response.hpp"
#include "../cgi/cgi.hpp"
#include "../response/StaticTarget.hpp"
#include "../response/delete.hpp"

void Client::HandleCgi(Request &req, const ServerConfig &server, Client *client, Epoll &epoll, const LocationConfig &loc)
{
    StaticTarget st;
    ResolvedTarget target = st.ResolveStaticTarget(req, server, loc);        
    if (target.status != 200)
    {
        client->sendError(target.status, target.reason, server);
        return;
    }
    req.setPath(target.path);
    Cgi cgi;
    cgi.handleCgi(req, server, client, epoll);
    client->setClientState(GENERATING_CGI);
    return ;    
}

void Client::HandlePost(Request &req, const ServerConfig &server, const LocationConfig &loc)
{
    std::cout << "UPLOAD" << std::endl; 
    Upload up;
    int status = up.CheckBodySize(loc, req);
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
        int upStatus = up.Handle(loc, req);
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
    sendError(400, "Bad Request", server);
}
void Client::HandleDelete(Request &req, const LocationConfig &loc, Client *client)
{
    req.displayRequest();

    Response res;
    // std::cout << "Let's delete this shit" << std::endl;
    // std::cout << "real http delete request" << std::endl;
    // req.displayRequest();
    Delete del;
    int hasBeenDeleted = del.isFileExisting(req, loc);
    client->getResponseBuffer().push_front(res.buildDeleteResponse(hasBeenDeleted).constructResponse());
    client->setResponseComplete(true);
    return ;
}

void    Client::Handle(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server, Client *client, Epoll &epoll)
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
    LocationConfig fallback;
    const LocationConfig *loc = req.MatchLocation(req.getPath(), locations, server, fallback);
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
        sendRedirect(loc->getRedirect());
        return ;
    }
    if (isCgi(req, server, *loc))
        return (HandleCgi(req, server, client, epoll, *loc));
    if (req.getMethod() == "POST")
        return (HandlePost(req, server, *loc));        
    else if (req.getMethod() == "DELETE")
        return (HandleDelete(req, *loc, client));
    StaticTarget st;
    ResolvedTarget target = st.ResolveStaticTarget(req, server, *loc);
    if (target.status == 301)
    {
        sendRedirect(target.path);
        return;
    }
    if (target.status != 200)
    {
        sendError(target.status, target.reason, server);
        return;
    }
    int stStatus = st.BuildStaticResponse(req, target, client, this->_response);
    if (stStatus != 200)
    {
        if (stStatus == 405)
            sendError(405, "Method Not Allowed", server);
        else
            sendError(403, "Forbidden", server);
        return;
    }
    if(this->_response.getResponseState() == FIRST_READ)
    {    
        client->getResponseBuffer().push_front(this->_response.constructResponse());
        this->_response.setResponseState(NEXT_READ);
    }
    else
    {          
        client->getResponseBuffer().push_front(this->_response.getBody());     
    }
    this->_response.getBody().clear();
}