#include "epoll.hpp"
# include "../response/Response.hpp"

void Epoll::creatEpollFdListeners(std::vector<int>& listener_fds)
{
    this->_ep_fd = epoll_create(10);
    for (unsigned int i = 0; i < listener_fds.size(); i++)
    {
        this->_ev.data.fd = listener_fds.at(i); 
        this->_ev.events = EPOLLIN;
        epoll_ctl(this->_ep_fd, EPOLL_CTL_ADD, listener_fds.at(i), &_ev);
    }
}

void Epoll::creactNewClient(std::vector<int>& listener_fds, int j)
{
    Client* client = new Client;
    client->getFd() = accept(listener_fds.at(j), NULL, NULL);
    Clients_map.insert(std::make_pair(client->getFd(), client));
    int flags = fcntl(Clients_map.at(client->getFd())->getFd(), F_GETFL, 0);
    fcntl(Clients_map.at(client->getFd())->getFd(), F_SETFL, flags | O_NONBLOCK);
    _ev.events = EPOLLIN;
    _ev.data.fd = Clients_map.at(client->getFd())->getFd();
    epoll_ctl(_ep_fd, EPOLL_CTL_ADD, Clients_map.at(client->getFd())->getFd(), &_ev);
}

void Epoll::HeaderEnd(Client *client, std::string bufferString)
{
    unsigned int found = bufferString.find("\r\n\r\n");                                     
    client->getRequestBuffer().append(bufferString.substr(0, found));                                                         
    client->getRequestClass().parseRequest(client->getRequestBuffer());
    client->getRequestBuffer().clear();
    if (bufferString.size() > found + 4) // a taffer quand on lira le body
    {
        // std::cout << "* on rajoute le reste du buffer au body *" << std::endl;
        // writeInAscii(bufferString);
        // std::cout << bufferString << std::endl;
        client->getRequestBuffer().append(bufferString.substr(found + 4, bufferString.size()));
    }
    client->setClientState(READING_BODY);
}

// fonction a call pour gerer EPOLLIN
void Epoll::manageClientRequest(Client *client, int byteReads, char *buf, std::vector<ServerConfig> servers, std::map<int, Cgi*> &_CgiMap)
{         
    std::string bufferString(buf, 0, byteReads);
    if ((bufferString.find("\r\n\r\n"))!=std::string::npos)
        HeaderEnd(client, bufferString);
    else if (client->getClientState() == WAITING || client->getClientState() == READING_HEADER)
    {
        client->getRequestBuffer().append(bufferString);
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
        if (client->getRequestBuffer().length() == client->getContentLength())
            std::cout << "post body is : " << client->getRequestBuffer() << std::endl;
        if (client->getRequestClass().getMethod() == "POST" && client->getRequestBuffer().length() < client->getContentLength()) 
            client->getRequestBuffer().append(bufferString);
        // else
        // {
            // client->get_requestBuffer().append(bufferString);
        // }
    }
    if (client->getReadyToWrite() == true) // client prêt a recevoir une reponse
    {
        //partie parissa qui recoit la recoit la requete complete et peut faire routing reponse
        Response Res = client->getRequestClass().Handle(client->getRequestClass(), servers[0].getLocations(), servers[0], _CgiMap);
        //Res.displayResponse();
        client->setResponseBuffer(Res.constructResponse());
        _ev.events = EPOLLOUT ;
        _ev.data.fd = client->getFd();            
        epoll_ctl(this->_ep_fd, EPOLL_CTL_MOD, client->getFd(), &_ev);

        // std::cout << "string response" << std::endl;
        // std::cout << responseString << std::endl;   

        //reponse basique automatique pour voir que ca marche
        // std::string response = "HTTP/1.0 200 OK\r\n\r\nHELLO";
        // write(client->getFd(), client->getResponseBuffer().c_str(), client->getResponseBuffer().size());
        // client->setReadyToWrite(false);
        // client->clearRequest(); 
        // close(client->getFd()); 
    }
}

void Epoll::epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> servers)
{
    creatEpollFdListeners(listener_fds);
    while (1)
    {
        // std::cout << "waiting request..." << std::endl;
        _event_wait = epoll_wait(_ep_fd, _events, 10, -1);
        for (int i = 0; i < _event_wait; i++)
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
                for (it = Clients_map.begin(); it != Clients_map.end(); ++it)
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
                size_t byteReads = recv(_events[i].data.fd, buf, sizeof(buf), 0);
                if (byteReads > 0)
                    manageClientRequest(Clients_map.at(_events[i].data.fd), byteReads, buf, servers, _CgiMap);
            }
        }
    }
    return ;
}
