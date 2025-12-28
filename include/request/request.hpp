#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "config.hpp"
# include <fstream>

// * INFO UTILE EN VRAC *
// Only PATCH, POST, and PUT requests have a body
//The presence of a message body in a request is signaled by a Content-Length or Transfer-Encoding header field

enum method
{
    GET,
    POST,
    DELETE
};

class request
{
    private:
        std::string _method; // GET, POST, DELETE (mandatory).... maybe put the enum instead of the string
        std::string _request_target; // element sur lequel va etre applique la methode
        std::string _path;
        std::string _query;
        std::string _protocol; // HTTP/1.1. en general
        std::map<std::string, std::string> _header;
        std::string _body; // pas sur de devoir le stocker comme cela
    public:
        request();
        request(std::string request);
        request(const request &obj);
        ~request();
        request& operator=(const request &obj);
        void parseRequest(std::string request);
        void parse_request_first_line(std::stringstream &stream);
        void parse_header(std::stringstream &stream);
        void display_request();
};

struct response
{
    std::string protocol; // HTTP/1.1. en general
    std::string status_code; // code de sortie par rapport a la request (200 OK, 404 not found)
    std::string reason_phrase; // optionel pour decrire le code de sortie.
    std::map<std::string, std::string> _header;
    std::string body; // pas sur de devoir le stocker comme cela    
};

#endif