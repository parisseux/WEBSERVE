#include "client.hpp"

bool Client::isUpload()
{
    std::string type;
    type = headerValue("Content-Type", this->getRequestClass());
    if (type.find("multipart/form-data")!=std::string::npos)
        return true;
    return false;
}

unsigned int Client::getContentLength()
{
    std::string length;
    unsigned int content_length;
    char *pEnd;
    length = headerValue("Content-Length", this->getRequestClass());
    content_length = std::strtoul(length.c_str(), &pEnd, 10);
	if (*pEnd != '\0' && *pEnd != '\r')
	{    
		std::cout << "Error on content lenght ending char" << std::endl;              
	}    
    return (content_length);
}

void Client::clearRequest()
{
    getRequestClass().getMethod().clear();
    getRequestClass().getRequestTarget().clear();
    getRequestClass().getPath().clear();
    getRequestClass().getQuery().clear();
    getRequestClass().getProtocol().clear();                     
    getRequestClass().getBody().clear();
    getRequestClass().getHeaders().clear();
}
