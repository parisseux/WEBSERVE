#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "config.hpp"
# include <fstream>

// * INFO UTILE EN VRAC *
// Only PATCH, POST, and PUT requests have a body
//The presence of a message body in a request is signaled by a Content-Length or Transfer-Encoding header field

enum method // a voir peut-etre a utliser
{
    GET,
    POST,
    DELETE
};

class Request
{
    private:
        std::string _method; // GET, POST, DELETE (mandatory).... maybe put the enum instead of the string
        std::string _request_target; // element sur lequel va etre applique la methode
        std::string _path;
        std::string _query;
        std::string _protocol; // HTTP/1.1. en general
        std::map<std::string, std::string> _header;
        std::string _body;
    public:
        Request();
        Request(std::string request);
        ~Request();
        void parseRequest(std::string request);
        void parse_request_first_line(std::stringstream &stream);
        void parse_header(std::stringstream &stream);
        void parse_body(std::stringstream &stream);
        void display_request();
        std::string& getMethod();
        std::string& getRequestTarget();
        std::string& getPath();
        std::string& getQuery();
        std::string& getProtocol();
        std::string& getBody();
        std::map<std::string, std::string>& getHeader();                        
};

#endif