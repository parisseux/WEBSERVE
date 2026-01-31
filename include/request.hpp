#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "../src/config/ConfigFile.hpp"
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
        std::string _requestTarget; // element sur lequel va etre applique la methode
        std::string _path;
        std::string _query;
        std::string _protocol; // HTTP/1.1. en general (parissa: selon le sujet plutot HTTP 1.0)
        std::map<std::string, std::string> _header;
        std::string _body;
    public:
        Request();
        Request(std::string request);
        ~Request();
        void parseRequest(std::string request);
        void parseRequestFirstLine(std::stringstream &stream);
        void parseHeader(std::stringstream &stream);
        void parseBody(std::stringstream &stream);

        //modifiable
        std::string& getMethod() { return _method; }
        std::string& getRequestTarget() { return _requestTarget; }
        std::string& getPath() { return _path; }
        std::string& getQuery() { return _query; }
        std::string& getProtocol(){ return _method; }
        std::string& getBody() { return _protocol; }
  
        //lecture seule
        const std::string& getMethod()  const { return _method; }
        const std::string& getTarget()  const { return _requestTarget; }
        const std::string& getPath()    const { return _path; }
        const std::string& getQuery()   const { return _query; }
        const std::string& getProtocol()const { return _protocol; }
        const std::map<std::string,std::string>& getHeaders() const { return _header; }
        const std::string& getBody()    const { return _body; }
        std::string getHeader(const std::string& k) const {
        std::map<std::string,std::string>::const_iterator it = _header.find(k);
        return (it == _header.end()) ? "" : it->second; }  
        
        void setMethod(std::string method);
        void setTarget(std::string target);
        void setPath(std::string path);
        void setQuery(std::string query);
        void setProtocol(std::string protocol);

        bool hasHeader(const std::string& k) const { return _header.find(k) != _header.end(); }
        void displayRequest();
};

#endif
