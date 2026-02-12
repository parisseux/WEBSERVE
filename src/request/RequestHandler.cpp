#include "Request.hpp"
#include "../cgi/cgi.hpp"
#include "../client/client.hpp"
#include "../response/Response.hpp"
#include "../response/StaticTarget.hpp"
#include "../socket/epoll.hpp"

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

// void Request::Handle(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server, Client *client, Epoll &epoll)
// {
//     int status = req.ValidateRequest(req);
//     if (status == 400)
//     {
//         client->getResponseBuffer().push_front(Response::Error(400, "400 Bad Request").constructResponse());       
//         return ; 
//     }
//     if (status == 501)
//     {
//         client->getResponseBuffer().push_front(Response::Error(501, "501 Not Implemented").constructResponse());      
//         return ;        
//     }
//     const LocationConfig *loc = req.MatchLocation(req.getPath(), locations);
//     if (!loc)
//     {
//         client->getResponseBuffer().push_front(Response::Error(500, "500 No location matched (unexpected)").constructResponse());    
//         return ;          
//     }
//     status = req.MethodAllowed(req, loc);
//     if (status == 405)
//     {
//         client->getResponseBuffer().push_front(Response::Error(405, "405 Method Not Allowed").constructResponse());       
//         return ; 
//     }
//     // buildRedirectResponse(loc);

//     // CGI handler va executer un script ou un process
//     if (isCgi(req, server, *loc))
//     {
//         Cgi cgi;
//         cgi.handleCgi(req, server, *loc, client, epoll);
//         return ;
//     }
//     //upload handler (="POST") va venir écrire dans un fichiers
//     // handleUpload(req, server, *loc);

//     // static handler va lire un fichier
//     //Ici on se charge de trouver la réponse quon doit envoyer au clients
//     StaticTarget st;
//     ResolvedTarget target = st.ResolveStaticTarget(req, server, *loc);
//     Response res = st.BuildStaticResponse(req, target, client);
//     client->getResponseBuffer().push_front(res.constructResponse().data());
// }