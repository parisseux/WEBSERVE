#include "../../include/webserv.hpp"

static int validateRequest(const Request &req)
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

static int MethodAllowed(const Request& req, const LocationConfig* loc)
{
    if (!loc) 
        return 0;
    std::string m = req.getMethod();
    if (!loc->hasAllowMethods)
        return 0;
    for (std::vector<std::string>::const_iterator it = loc->allowMethods.begin();
         it != loc->allowMethods.end();
         ++it)
    {
        if (m == *it)
            return 0;
        if (m == "HEAD" && *it == "GET")
            return 0;
    }
    return 405;
}

static bool startsWith(const std::string& s, const std::string& prefix)
{
    if (prefix.size() > s.size())
        return false;
    if (s.compare(0, prefix.size(), prefix) != 0)
        return false;
    return true;
}

static const LocationConfig *MatchLocation(const std::string &reqLoc, const std::vector<LocationConfig> &locations)
{
    const LocationConfig* bestLoc = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < locations.size(); ++i)
    {
        const LocationConfig& loc = locations[i];
        if (startsWith(reqLoc, loc.path) && loc.path.size() > bestLen)
        {
            bestLoc = &loc;
            bestLen = loc.path.size();
        }
    }
    return (bestLoc);
}

Response HandleRequest(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server)
{
    int status = validateRequest(req);
    if (status == 400)
        return (Response::Error(400, "400 Bad Request"));
    if (status == 501)
        return (Response::Error(501, "501 Not Implemented"));
    const LocationConfig *loc = MatchLocation(req.getPath(), locations);
    if (!loc)
        return (Response::Error(500, "500 No location matched (unexpected)"));
    status = MethodAllowed(req, loc);
    if (status == 405)
        return (Response::Error(405, "405 Method Not Allowed"));

    // buildRedirectResponse(loc);

    //CGI handler va executer un script ou un process
    if (isCgi(req, server, *loc))  
        return (handleCgi(req, server, *loc));

    //upload handler (="POST") va venir écrire dans un fichier
    // handleUpload(req, server, *loc);

    // static handler va lire un fichier
    //Ici on se charge de trouver la réponse quon doit envoyer au clients
    ResolvedTarget target = resolveStaticTarget(req, server, *loc);
    return (buildStaticResponse(req, target));
}


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