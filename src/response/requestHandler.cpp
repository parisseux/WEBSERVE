#include "requestHandler.hpp"

RequestHandler::RequestHandler(Request& req, const std::vector<LocationConfig>& locations, const ServerConfig& server)
    : _req(req),
      _locations(locations),
      _server(server),
      _loc(NULL)
{ }

int RequestHandler::validateRequest() const
{
    if (_req.getMethod().empty())
        return 400;
    if (_req.getPath().empty() || _req.getPath()[0] != '/')
        return 400;
    if (_req.getProtocol() != "HTTP/1.1")
        return 400;
    if (_req.getMethod() != "GET" && _req.getMethod() != "POST"
        && _req.getMethod() != "DELETE" && _req.getMethod() != "HEAD")
        return 501;
    return (0);
}

int RequestHandler::methodAllowed() const
{
    if (!_loc || !_loc->hasAllowMethods)
        return 0;

    const std::string& m = _req.getMethod();
    for (size_t i = 0; i < _loc->allowMethods.size(); ++i)
    {
        if (m == _loc->allowMethods[i])
            return 0;
        if (m == "HEAD" && _loc->allowMethods[i] == "GET")
            return 0;
    }
    return (405);
}

bool RequestHandler::startsWith(const std::string& s, const std::string& prefix) const
{
    return (s.compare(0, prefix.size(), prefix) == 0);
}

const LocationConfig* RequestHandler::matchLocation() const
{
    const LocationConfig* best = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < _locations.size(); ++i)
    {
        const LocationConfig& loc = _locations[i];
        if (startsWith(_req.getPath(), loc.path)
            && loc.path.size() > bestLen)
        {
            best = &loc;
            bestLen = loc.path.size();
        }
    }
    return (best);
}

int RequestHandler::checkBodySize() const
{
    if (!_loc->hasMaxBodySize)
        return 0;
    if (_req.getBody().size() > _loc->_maxBodySize)
        return 413;
    return (0);
}

Response RequestHandler::handle()
{
    std::cout << "PATH = [" << _req.getPath() << "]" << std::endl;
    int status = validateRequest();
    if (status)
        return Response::Error(status, "Request Error");

    _loc = matchLocation();
    if (!_loc)
        return Response::Error(500, "No location matched");

    status = methodAllowed();
    if (status)
        return Response::Error(405, "Method Not Allowed");

    if (_req.getMethod() == "POST")
    {
        status = checkBodySize();
        if (status)
            return Response::Error(413, "Payload Too Large");
        if (!_req.hasHeader("Content-Type") || !_req.hasHeader("Content-Length"))
            return Response::Error(400, "Bad Request");
        if (_req.getHeader("Content-Type").rfind("multipart/form-data", 0) == 0
            && _req.getPath() == "/upload")
        {
            HandleUpload uploader;
            return uploader.handle(_req, *_loc);
        }

        return Response::Error(415, "Unsupported Media Type");
    }

    ResolvedTarget target = StaticTarget::resolve(_req, _server, *_loc);
    return (StaticTarget::buildResponse(_req, target));
}

// Response HandleRequest(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server)
// {
//     std::cout << "lala" << std::endl;
//     //req.displayRequest();
//     int status = validateRequest(req);
//     if (status == 400)
//         return (Response::Error(400, "400 Bad Request"));
//     if (status == 501)
//         return (Response::Error(501, "501 Not Implemented"));
//     const LocationConfig *loc = MatchLocation(req.getPath(), locations);
//     if (!loc)
//         return (Response::Error(500, "500 No location matched (unexpected)"));
//     status = MethodAllowed(req, loc);
//     if (status == 405)
//         return (Response::Error(405, "405 Method Not Allowed"));
//     if (req.getMethod() == "POST")
//         status = checkBodySize(req, loc);
//     if (status == 413)
//         return (Response::Error(413, "413 Payload  Too Large"));

//     // buildRedirectResponse(loc);
//     //CGI handler va executer un script ou un process
//     // handleCgi(req, server, *loc);
//     //upload handler (="POST") va venir écrire dans un fichier
//     if (req.getMethod() == "POST")
//     {
//         if (!req.hasHeader("Content-Type") || !req.hasHeader("Content-Length"))
//             return (Response::Error(400, "400 Bad Request"));
        
//         // if (isCGI(req.getTarget(), locations))
//         //     return HandleCGI(request);
        
//         std::cout << "Lets post some stufff" << std::endl;
//         // return (Response::Error(415, "Unsupported Media Type"));
//         if (req.getHeader("Content-Type").rfind("multipart/form-data", 0) == 0 && req.getPath() == "/upload")
//         {
//             HandleUpload uploader;
//             return (uploader.handle(req, *loc));
//         }
//     }

    // static handler va lire un fichier
    //Ici on se charge de trouver la réponse quon doit envoyer au clients
//     ResolvedTarget target = StaticTarget::resolve(req, server, *loc);
//     return (StaticTarget::buildResponse(req, target));
// }


//-----------THEORY------------
//cours sur les handler --> le handler est comme un "cerveau" spécialisé pour chaque cas.
//le server va recevoir une requete abstraite et il doit répondre selon un contexte
//et cest la que les handlers vont rentrer en compte

//1. static handler --> pour les fichiers statiques
//cest le handler le plus fondamentale (90% des reque^tes web passent ici)
//il peut servir plsuiques type de fichiers présents sur le disque au clients
//.html, .css, .js, images, .txt, ....
//il sers les méthode GET et HEAD
//réponse possible: 
    //200 (fichier trouvé)
    //403 (dossier sans index)
    //404 fichier inexistant
    //403 (permission refusée)
    //HEAD --> 200 mais sans body 


//2. CGI handler --> va permettre d'executer des programmes
//la réponse ici est dynamique
// le server peut executer un programme externe, c'est-à-dique 
//que le server ne connait pas le contenu il va simplement le délégué aux clients
//ici ce handler va traiter méthode GET/POST, scripts exécutables, variables d'environnement, stdin/stdout
//réponse possible:
    //200 (script ok)
    //500 (script crash)
    //504 timeout 
    //403 (script non exécutable)

//plus trop utilisé aujo car mnt y a FastCGI qui la remplacé

//Upload handler --> permet décrire sur le disque 
//en gros on prend des données envoyé par le client et on les écrit sur le server
//exemple: upload d'image, formulaire (style login avec password), fichier texte, data "POST"
//ici on traite: méthode "POST", body HTTP, 
//réponse possible:
    //201 (uplaod réussi)
    //400 (pas de body)
    //413 taille trop grande
    //405 méthode intédite

//4. Redirect handler --> headers only
//ici le handler dit au client que la ressouce se trouve ailleir
//pas de body nécessaire
//il traite n'importe quelle méthode 
//réponse possible:
    //301 (redirection permanente)
    //302 temporaire 
    //307 (stricte)