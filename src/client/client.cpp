#include "client.hpp"


void Client::setFd(int fd)
{
    this->_fd = fd;
}

void Client::setFlags(int flags)
{
    this->_flags = flags;
}

void Client::setRequestBuffer(std::string requestBuffer)
{
    this->_requestBuffer = requestBuffer;
}

void Client::setResponseBuffer(std::string responseBuffer)
{
    this->_responseBuffer = responseBuffer;
}

void Client::setReadyToWrite(bool ReadytoWrite)
{
    this->_ReadyToWrite = ReadytoWrite;
}

void Client::setClientState(ClientState state)
{
    this->_state = state;
}

// ALL THE GETTERS

int& Client::getFd()
{
    return (this->_fd);
}

int& Client::getFlags()
{
    return (this->_flags);
}

std::string& Client::getRequestBuffer()
{
    return (this->_requestBuffer);
}

std::string& Client::getResponseBuffer()
{
    return (this->_responseBuffer);
}

bool& Client::getReadyToWrite()
{
    return (this->_ReadyToWrite);
}

Request& Client::getRequestClass()
{
    return (this->_request);
}

ClientState Client::getClientState()
{
    return(this->_state);
}

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
