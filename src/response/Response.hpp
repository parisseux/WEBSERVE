#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <fstream>
# include <string>
# include <fcntl.h>
# include <cstdlib>
# include <arpa/inet.h>
# include <vector>
# include <sstream>
# include <map>
# include <sys/stat.h>
# include <fstream>
# include <sstream>
# include "../request/Request.hpp"

enum TargetType {
    FILE_TARGET,
    AUTO_INDEX_TARGET
};

struct ResolvedTarget {
    int status;
    std::string path;
    struct stat st;
    std::string reason;
    TargetType type;
};

enum ResponseState
{
    FIRST_READ,
    NEXT_READ
};

class Response 
{
    private:
        int _status;
        std::string _statusLine;
        std::map<std::string, std::string> _headers;
        std::string _body;
        ResponseState _state;
    public:
        Response() : _status(200), _statusLine("HTTP/1.1 200 OK"), _state(FIRST_READ){}
        ~Response() {}

        void setStatus(int code);
        void setHeader(const std::string& k, const std::string& v);
        void setBody(const std::string& b);
        void setResponseState(ResponseState state){_state = state;}        
        static std::string makeStatusLine(int code);

        int getStatus() { return _status; }
        std::string& getStatusLine() { return _statusLine; }
        std::map<std::string, std::string>& getHeaders() { return _headers; }
        std::string& getBody() { return _body; }
        ResponseState& getResponseState(){return _state;}
        
        static Response Error(int code, const std::string &s);
        void displayResponse();
        std::string constructResponse();
        Response buildDeleteResponse(int hasBeenDeleted);
        std::string addBodyToResponseBuffer();
        ssize_t getContentLength();
};

#endif
