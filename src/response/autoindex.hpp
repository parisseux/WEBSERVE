#ifndef AUTOINDEX_HPP
# define AUTOINDEX_HPP

# include <iostream>
# include <string>
# include <dirent.h>
# include <sys/stat.h>
# include <sstream>
# include <vector>
# include <algorithm>

struct ResolvedTarget;
class Request;

// diskPath   = chemin sur disque du dossier (ex: /var/www/site/images)
//--> ca va nous permettre de lire sur le disque
//---> vient de la config root /www/
// requestPath = chemin URL demandé (ex: /images/)
//--> ca va nous permettre de construire les liens HTTP
//--> vient du client grâce à la requete HTTP envoyer au server


//exemple configfile 04: si client demande /
//root = "./www/listing"
//req.getPath() = "/"
//loc.getPath() = "/"
//relativ path = ""
//diskPath = JoinPath("./www/listing", "")  = "./www/listing"
//pas de index.html mais autoindex on DONC
// GenerateAutoIndexHtml("./www/listing", "/")


//stock une entrée du dossier
//jai pas fait de classe pck autoindex "stateless" donc en vrai jcrois pas nécessaire
struct AutoIndexEntry
{
    std::string name;
    bool isDir;
    AutoIndexEntry() : name(), isDir(false) {}
    AutoIndexEntry(const std::string& n, bool isDir) : name(n), isDir(isDir) {}
};

std::string GenerateAutoIndexHtml(const std::string &diskPath, const std::string &reqPath);

#endif
