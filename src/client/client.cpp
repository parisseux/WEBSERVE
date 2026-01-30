#include "client.hpp"

// CONSTRUCTOR / DESTRUCTOR

Client::Client()
: _ReadyToWrite(false), _state(WAITING)
{
    std::cout << "Client created" << std::endl;
}

Client::~Client()
{
    std::cout << "Client destructed" << std::endl;
}

// ALL THE SETTESS

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

int Client::getContentLength()
{
    std::string length;
    int content_length;
    char *pEnd;
    length = headerValue("Content-Length", this->getRequestClass());
    content_length = std::strtol(length.c_str(), &pEnd, 0);
	if (*pEnd != '\0')
	{    
		std::cout << "Erreur conversion en int dans calcul content length" << std::endl;              
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
