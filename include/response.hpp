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
    int getStatus() const { return _status; }
    const std::string& getStatusLine() const { return _statusLine; }
    const std::map<std::string, std::string>& getHeaders() const { return _headers; }
    const std::string& getBody() const { return _body; }
    static std::string makeStatusLine(int code);
    static Response Error(int code, const std::string &s);
    void display_response();
    std::string constructResponse();
};

#endif
