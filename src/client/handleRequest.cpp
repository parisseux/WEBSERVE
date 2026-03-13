#include "client.hpp"
#include "../response/Response.hpp"
#include "../cgi/cgi.hpp"
#include "../response/StaticTarget.hpp"
#include "../response/delete.hpp"

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
        StaticTarget st;
        ResolvedTarget target = st.ResolveStaticTarget(req, server, *loc);        
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
    if (req.getMethod() == "POST")
    {
        std::cout << "UPLOAD" << std::endl; 
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
    else if (req.getMethod() == "DELETE")
    {
        req.displayRequest();

        Response res;
        // std::cout << "Let's delete this shit" << std::endl;
        // std::cout << "real http delete request" << std::endl;
        // req.displayRequest();
        Delete del;
        int hasBeenDeleted = del.isFileExisting(req);
        client->getResponseBuffer().push_front(res.buildDeleteResponse(hasBeenDeleted).constructResponse());
        client->setResponseComplete(true);
        return ;
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
        client->getResponseBuffer().push_front(_response.constructResponse());
        _response.setResponseState(NEXT_READ);
    }
    else
    {          
        client->getResponseBuffer().push_front(_response.getBody());     
    }
    _response.getBody().clear();
}