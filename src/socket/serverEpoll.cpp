#include "epoll.hpp"
# include "../response/Response.hpp"
# include "../utils/utils.hpp"

void print_ready_events(int num_events, struct epoll_event* events_array) {
	// std::cout << "Nombre d'événements détectés : " << num_events << std::endl;

	for (int i = 0; i < num_events; ++i) {
		// On vérifie si le flag EPOLLIN est présent dans le champ events
		if (events_array[i].events & EPOLLIN) {
			std::cout << "  [FD " << events_array[i].data.fd 
					  << "] est prêt pour la lecture (EPOLLIN)" << std::endl;
		}
		if (events_array[i].events & EPOLLOUT) {
			std::cout << "  [FD " << events_array[i].data.fd 
					  << "] est prêt pour la lecture (EPOLLOUT)" << std::endl;
		}
		// Optionnel : vérifier aussi les erreurs ou déconnexions
		if (events_array[i].events & EPOLLERR) {
			std::cerr << "  [FD " << events_array[i].data.fd << "] Erreur détectée !" << std::endl;
		}
	}
}

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
    client->setRequestComplete(false);
}

void Epoll::HeaderEnd(Client *client)
{
    std::cout << "HEADER END" << std::endl;
    unsigned int found = client->getRequestBuffer().find("\r\n\r\n");                                                                                      
    client->getRequestClass().parseRequest(client->getRequestBuffer());
    client->getRequestBuffer().erase(0, found + 4);
    client->setClientState(READING_BODY);
}

// fonction a call pour gerer EPOLLIN
void Epoll::manageClientRequest(Client *client, ssize_t byteReads, char *buf)
{
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
        if (client->getRequestClass().getMethod() == "GET"
			|| client->getRequestClass().getMethod() == "DELETE")
        {
            client->setClientState(WAITING);
            client->setRequestComplete(true);
            // client->getRequestClass().displayRequest(); // affichage requete complete
        }
        if (client->getRequestClass().getMethod() == "POST")
        {
            if(client->getRequestBuffer().size() >= client->getContentLength())
            {
                client->getRequestClass().parseBody(client);   
            }                   
        }
    }    
    if (client->getRequestComplete() == true) // client prêt a recevoir une reponse
    {
		client->setClientState(GENERATING_RESPONSE);

    }
}

void Epoll::manageCgi(Client *client, int byteReads, char *buf)
{
	// std::cout << "MANAGE CGI" << std::endl;
	std::string bufferString(buf, byteReads);
	client->getResponseBuffer().push_back(bufferString);
    client->setResponseComplete(false);		
	if ((bufferString.find("0\r\n\r\n"))!=std::string::npos)
	{
		// std::cout << "DELETE AND CLOSE CGI FD" <<  std::endl;
		epoll_ctl(this->_epFd, EPOLL_CTL_DEL, client->getCgiFd(), &_ev);    
        client->setResponseComplete(true);		
		close(client->getCgiFd());  
	}
}

void Epoll::NewClientConnection(std::vector<int>& listener_fds, int eventFd)
{
	for (unsigned int j = 0; j < listener_fds.size(); j++) //cree un nouveau client en cas de nouvelle connexion
	{
		if (eventFd == listener_fds.at(j))
		{
			creactNewClient(listener_fds, j);
			_is_listener = true;
			break ;
		}
	}
}

void Epoll::MatchEventWithClient(int eventFd)
{
	for (_it = _clientsMap.begin(); _it != _clientsMap.end(); ++_it) // choisi le bon client en fonction du fd de l'event recu
	{
		if (eventFd == _it->first)
		{
			_client = _it->second;
			_isCgi = false;
			break ;
		}
		_client_cgi = _it->second;
		if (eventFd == _client_cgi->getCgiFd())
		{
			_isCgi = true;
			break;
		}
	}		
}

void Epoll::HandleEpollin(int eventFd)
{
	char buf[4000];
	ssize_t byteReads = read(eventFd, buf, sizeof(buf));
	if (byteReads > 0)
	{
		// std::cout << byteReads << std::endl; 
		if (_isCgi == true)
			manageCgi(_client_cgi, byteReads, buf);
		else
			manageClientRequest(_clientsMap.at(eventFd), byteReads, buf);
	}	
}

void Epoll::HandleEpollout()
{
	if (_client->getResponseBuffer().empty() == 0)
	{
		std::cout << "MESSAGE ENVOYE" << std::endl;
		std::cout << _client->getResponseBuffer().front() << std::endl;
		std::string response = _client->getResponseBuffer().front();
		_client->getResponseBuffer().pop_front();
		ssize_t byteReads = send(_client->getFd(), response.c_str(), response.size(), 0);
		if (byteReads > 0)
		{
			// std::cout << "send some byte" << std::endl;
		}
		if (_client->getResponseComplete() == true) // a voir mettre secu en plus car fonction send envoie ce qu'il veut 
		{
			std::cout << "send finished" << std::endl;                        
			_ev.events = EPOLLIN ;
			_ev.data.fd = _client->getFd();            
			epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);
			_client->clearClient();
			// _clientsMap.erase(client->getFd()); // dans le cas d'un NON keep alive
			// delete client;											
		}
	}
}

void Epoll::generatePendingResponse(std::vector<ServerConfig> &servers)
{
	for (_it = _clientsMap.begin(); _it != _clientsMap.end(); ++_it)
	{
		//std::cout << "MAP CLIENTS SIZE: " << _clientsMap.size() << std::endl;
		_client = _it->second;
		if(_client->getClientState() == GENERATING_RESPONSE)
		{
			_client->Handle(_client->getRequestClass(),  servers[0].getLocations(), servers[0], _client, *this);	
			// client->setClientState(SENDING_RESPONSE);				
			_ev.events = EPOLLOUT ;
			_ev.data.fd = _client->getFd();            
			epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);
		}
	}
}

void Epoll::epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> &servers)
{
	creatEpollFdListeners(listener_fds);
	while (1)
	{
		// std::cout << "waiting request..." << std::endl;
		_eventWait = epoll_wait(_epFd, _events, 10, -1);
		// print_ready_events(_event_wait, _events);
		for (int i = 0; i < _eventWait; i++)
		{
			_isCgi = false;
			_is_listener = false;
			NewClientConnection(listener_fds, _events[i].data.fd);
			MatchEventWithClient(_events[i].data.fd);
			if (!_is_listener && (_events[i].events & EPOLLIN))
				HandleEpollin(_events[i].data.fd);
			else if (!_is_listener && (_events[i].events & EPOLLOUT) && _isCgi == false)
				HandleEpollout();
			generatePendingResponse(servers);					
		}
	}
	return ;
}
