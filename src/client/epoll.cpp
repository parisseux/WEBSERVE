#include "webserv.hpp"

Epoll::Epoll()
{
    std::cout << "Epoll constructor called" << std::endl;
}

Epoll::~Epoll()
{
    std::cout << "Epoll destructor called" << std::endl;
}

// EPOLL SETTERS

void Epoll::set_epfd(int ep_fd)
{
    this->_ep_fd = ep_fd;
}

void Epoll::set_event_wait(int event_wait)
{
    this->_event_wait = event_wait;
}

// EPOLL GETTERS

int& Epoll::get_epfd()
{
    return (this->_ep_fd);
}

int& Epoll::get_event_wait()
{
    return (this->_event_wait);
}
