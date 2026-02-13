#include "epoll.hpp"
# include "../response/Response.hpp"

void Epoll::creatEpollFdListeners(std::vector<int>& listener_fds)
{
    this->_epFd = epoll_create(10);
    for (unsigned int i = 0; i < listener_fds.size(); i++)
    {
        this->_ev.data.fd = listener_fds.at(i); 
        this->_ev.events = EPOLLIN;
        epoll_ctl(this->_epFd, EPOLL_CTL_ADD, listener_fds.at(i), &_ev);
    }
}

void Epoll::creactNewClient(std::vector<int>& listener_fds, int j)
{
    Client* client = new Client;
    client->getFd() = accept(listener_fds.at(j), NULL, NULL);
    _clientsMap.insert(std::make_pair(client->getFd(), client));
    int flags = fcntl(_clientsMap.at(client->getFd())->getFd(), F_GETFL, 0);
    fcntl(_clientsMap.at(client->getFd())->getFd(), F_SETFL, flags | O_NONBLOCK);
    _ev.events = EPOLLIN;
    _ev.data.fd = _clientsMap.at(client->getFd())->getFd();
    epoll_ctl(this->_epFd, EPOLL_CTL_ADD, _clientsMap.at(client->getFd())->getFd(), &_ev);
    client->setClientState(WAITING);
    client->setReadyToWrite(false);
}

void Epoll::HeaderEnd(Client *client)
{
    std::cout << "HEADER END" << std::endl;
    unsigned int found = client->getRequestBuffer().find("\r\n\r\n");                                                                                      
    client->getRequestClass().parseRequest(client->getRequestBuffer());
    client->getRequestBuffer().erase(0, found + 4);
    
    // client->getRequestClass().displayRequest();
    // std::cout << "jme sens comme avant la fusdion" << std::endl;
    // std::cout << client->getRequestBuffer() << std::endl;
    client->setClientState(READING_BODY);
}

// fonction a call pour gerer EPOLLIN
void Epoll::manageClientRequest(Client *client, ssize_t byteReads, char *buf, std::vector<ServerConfig> &servers, std::map<int, Cgi*> &_CgiMap)
{
    // std::cout << "BUFFER DU RECV" << std::endl;
    // std::cout << buf << std::endl;
    std::string bufferString(buf, byteReads);
    client->getRequestBuffer().append(bufferString);
    if (client->getClientState() == WAITING || client->getClientState() == READING_HEADER)
    {
        size_t pos = client->getRequestBuffer().find("\r\n\r\n");
        bool hasDelimiter = (pos !=std::string::npos); 
        if (hasDelimiter && (client->getClientState() == READING_HEADER || client->getClientState() == WAITING))
            HeaderEnd(client);
        if (client->getClientState() != READING_BODY)
            client->setClientState(READING_HEADER);
    }    
    if (client->getClientState() == READING_BODY)
    {
        if (client->getRequestClass().getMethod() == "GET")
        {
            client->setClientState(WAITING);
            client->setReadyToWrite(true);
            //client->getRequestClass().displayRequest(); // affichage requete complete
        }
        if (client->getRequestClass().getMethod() == "POST")
        {
            if(client->getRequestBuffer().size() >= client->getContentLength())
            {
                client->getRequestClass().parseBody(client);   
            }            
            // std::cout << "APPEND to the POST" << std::endl;           
            // client->getRequestClass().getBodyBinary().insert(client->getRequestClass().getBodyBinary().end(), )
            // std::cout << client->getContentLength() << std::endl;
            // std::cout << client->getRequestBuffer().size() << std::endl;             
        }
    }    
    if (client->getReadyToWrite() == true) // client prêt a recevoir une reponse
    {
        // client->getRequestClass().displayRequest();
        Response Res = client->getRequestClass().Handle(client->getRequestClass(), servers[0].getLocations(), servers[0], _CgiMap);
        client->setResponseBuffer(Res.constructResponse());
        _ev.events = EPOLLOUT ;
        _ev.data.fd = client->getFd();
        epoll_ctl(this->_epFd, EPOLL_CTL_MOD, client->getFd(), &_ev);
    }
}

void Epoll::epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> &servers)
{
    creatEpollFdListeners(listener_fds);
    while (1)
    {
        // std::cout << "waiting request..." << std::endl;
        _eventWait = epoll_wait(_epFd, _events, 10, -1);
        for (int i = 0; i < _eventWait; i++)
        {
            bool is_listener = false;
            for (unsigned int j = 0; j < listener_fds.size(); j++)
            {
                if (_events[i].data.fd == listener_fds.at(j))
                {
                    creactNewClient(listener_fds, j);
                    is_listener = true;
                    break;
                }
            }
            if (!is_listener && (_events[i].events & EPOLLOUT))
            {
                std::map<int, Client*>::iterator it;
                for (it = _clientsMap.begin(); it != _clientsMap.end(); ++it)
                {
                    //std::cout << it->first << std::endl;
                    if (_events[i].data.fd == it->first)
                    {
                        //std::cout << "client found" << std::endl;
                        Client *client;
                        client = it->second;
                        size_t byteReads = send(_events[i].data.fd, client->getResponseBuffer().c_str(), client->getResponseBuffer().size(), 0);
                        if (byteReads > 0)
                            std::cout << "send finished" << std::endl;
                        client->setReadyToWrite(false);
                        client->clearRequest(); 
                        close(client->getFd());
                    }
                }
            }                
            if (!is_listener && (_events[i].events & EPOLLIN))
            {
                char buf[4000];
                ssize_t byteReads = recv(_events[i].data.fd, buf, sizeof(buf), 0);
                // std::cout << "BUFFER DU RECV" << std::endl;
                // std::cout << buf << std::endl;
                // std::cout << "FIN DU RECV" << std::endl;                
                if (byteReads > 0)
                    manageClientRequest(_clientsMap.at(_events[i].data.fd), byteReads, buf, servers, _CgiMap);
            }
        }
    }
    return ;
}

    // if (client->getClientState() == READING_BODY)
    // {
    //     const std::string &cl = client->getRequestClass().getHeader("Content-Length");
    //     unsigned long contentLength = 0;

    //     if (!cl.empty())
    //     {
    //         char *endptr = NULL;
    //         contentLength = std::strtoul(cl.c_str(), &endptr, 10);
    //         if (*endptr != '\0')
    //             contentLength = 0;
    //     }

    //     size_t headersEnd = client->getRequestBuffer().find("\r\n\r\n");
    //     size_t bodyStart = headersEnd + 4;
    //     size_t bodySize = client->getRequestBuffer().size() - bodyStart;

    //     if (bodySize >= contentLength)
    //     {
    //         std::string fullRequest = client->getRequestBuffer();
    //         client->getRequestClass().parseRequest(client->getRequestBuffer());
    //         client->setClientState(WAITING);
    //         client->setReadyToWrite(true);
    //     }
    //     else
    //     {
    //         return;
    //     }
    // }










