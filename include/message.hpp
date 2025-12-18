# include "config.hpp"

// * INFO UTILE EN VRAC *
// Only PATCH, POST, and PUT requests have a body
//The presence of a message body in a request is signaled by a Content-Length or Transfer-Encoding header field


struct header
{
    std::map<std::string, std::string> header;
};

struct request
{
    std::string method; // GET, POST, PUT....
    std::string request_target; // element sur lequel va etre applique la methode
    std::string protocol; // HTTP/1.1. en general
    header header;
    std::string body; // pas sur de devoir le stocker comme cela
};

struct response
{
    std::string protocol; // HTTP/1.1. en general
    std::string status_code; // code de sortie par rapport a la request (200 OK, 404 not found)
    std::string reason_phrase; // optionel pour decrire le code de sortie.
    header header;
    std::string body; // pas sur de devoir le stocker comme cela    
};


