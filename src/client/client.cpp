#include "webserv.hpp"

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

void Client::set_fd(int fd)
{
    this->_fd = fd;
}

void Client::set_flags(int flags)
{
    this->_flags = flags;
}

void Client::set_requestBuffer(std::string requestBuffer)
{
    this->_requestBuffer = requestBuffer;
}

void Client::set_responseBuffer(std::string responseBuffer)
{
    this->_responseBuffer = responseBuffer;
}

void Client::set_ReadyToWrite(bool ReadytoWrite)
{
    this->_ReadyToWrite = ReadytoWrite;
}

void Client::set_clientState(ClientState state)
{
    this->_state = state;
}

// ALL THE GETTERS

int& Client::get_fd()
{
    return (this->_fd);
}

int& Client::get_flags()
{
    return (this->_flags);
}

std::string& Client::get_requestBuffer()
{
    return (this->_requestBuffer);
}

std::string& Client::get_responseBuffer()
{
    return (this->_responseBuffer);
}

bool& Client::get_ReadyToWrite()
{
    return (this->_ReadyToWrite);
}

Request& Client::get_requestClass()
{
    return (this->_request);
}

ClientState Client::get_clientState()
{
    return(this->_state);
}
