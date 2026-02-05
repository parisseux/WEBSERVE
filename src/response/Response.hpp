#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <fstream>
# include <string>
# include <cstdlib>
# include <arpa/inet.h>
# include <vector>
# include <sstream>
# include <map>
# include <sys/stat.h>
# include <fstream>
# include <sstream>

# include "../request/Request.hpp"

struct ResolvedTarget {
    int status;
    std::string path;
    struct stat st;
    std::string reason;
};

//------THEORY-----
//anatomy of HTTP response 
//stauts code: tell the client what happened with the request
//ici le body contiens les data que le clients a demandé
//body optional mais usually present

//concept statlessness: each request is independant of the others
//no request is aware of what other request did before

class Response 
{
private:
    int _status;
    std::string _statusLine;
    std::map<std::string, std::string> _headers;
    std::string _body;

public:
    Response() : _status(200), _statusLine("HTTP/1.1 200 OK") {}
    ~Response() {}

    void setStatus(int code);
    void setHeader(const std::string& k, const std::string& v);
    void setBody(const std::string& b);

    int getStatus() { return _status; }
    std::string& getStatusLine() { return _statusLine; }
    std::map<std::string, std::string>& getHeaders() { return _headers; }
    std::string& getBody() { return _body; }

    static std::string makeStatusLine(int code);
    static Response Error(int code, const std::string &s);
    void displayResponse();
    std::string constructResponse();
    // Response buildUploadResponse(const std::string file);
};

#endif
