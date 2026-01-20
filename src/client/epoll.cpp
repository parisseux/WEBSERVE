#include "../../include/webserv.hpp"

Epoll::Epoll()
{
    std::cout << "Epoll constructor called" << std::endl;
}

Epoll::~Epoll()
{
    std::cout << "Epoll destructor called" << std::endl;
}

// EPOLL SETTERS

void Epoll::setEpFd(int ep_fd)
{
    this->_ep_fd = ep_fd;
}

void Epoll::setEventWait(int event_wait)
{
    this->_event_wait = event_wait;
}

// EPOLL GETTERS

int& Epoll::getEpFd()
{
    return (this->_ep_fd);
}

int& Epoll::getEventWait()
{
    return (this->_event_wait);
}
