#include "epoll.hpp"
# include "../response/Response.hpp"
# include "../utils/utils.hpp"

void print_ready_events(int num_events, struct epoll_event* events_array) {
    std::cout << "Nombre d'événements: " << num_events << std::endl;
    for (int i = 0; i < num_events; ++i) {
        std::cout << "  [FD " << events_array[i].data.fd << "] events=";
        if (events_array[i].events & EPOLLIN)
            std::cout << "EPOLLIN ";
        if (events_array[i].events & EPOLLOUT)
            std::cout << "EPOLLOUT ";
        if (events_array[i].events & EPOLLHUP)
            std::cout << "EPOLLHUP ";
        if (events_array[i].events & EPOLLRDHUP)
            std::cout << "EPOLLRDHUP ";
        if (events_array[i].events & EPOLLERR)
            std::cout << "EPOLLERR ";
        std::cout << std::endl;
    }
}

void	Epoll::printClientMap()
{
	std::cout << "Size of clients Map: " << _clientsMap.size() << std::endl;
	for (_it = _clientsMap.begin();_it != _clientsMap.end(); ++_it)
	{
		std::cout << "client of fd: " << _it->first << " connected to server: " << _it->second->getServerIndex () << std::endl;
	}
}

void Epoll::creatEpollFdListeners(std::vector<int>& listener_fds)
{
    this->_epFd = epoll_create(MAX_CLIENTS);
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
	int newFd = accept(listener_fds.at(j), NULL, NULL);
	if (newFd < 0)
		throw std::runtime_error("ACCEPT function failed when creating a new client\n");
    client->setFd(newFd);
	client->setServerIndex(j);
    int flags = fcntl(client->getFd(), F_GETFL, 0);
	if (flags == -1)
	{
		close (newFd);
		throw std::runtime_error("FCTNL function failed to create a flag\n");
	}
    if (fcntl(client->getFd(), F_SETFL, flags | O_NONBLOCK) == -1)
	{
		close(newFd);
		throw std::runtime_error("FCTNL function failed to making flag NONBLOCK\n");
	}			
    _ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR;
    _ev.data.fd = client->getFd();
    epoll_ctl(this->_epFd, EPOLL_CTL_ADD, client->getFd(), &_ev);
	_clientsMap.insert(std::make_pair(client->getFd(), client));
}

void Epoll::HeaderEnd(Client *client)
{
    // std::cout << "HEADER END" << std::endl;
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
		client->getRequestClass().displayRequest(); // affichage requete complete
		client->setClientState(GENERATING_RESPONSE);
    }
}

void Epoll::formatingchunk(Client *client, std::string bufferString)
{
	if (bufferString.empty())
		return; 
	std::stringstream ss;
	ss << std::hex << bufferString.size();
	std::string chunk;
	chunk.append(ss.str());
	chunk.append("\r\n");
	chunk.append(bufferString);
	chunk.append("\r\n");
	client->getResponseBuffer().push_back(chunk);
}

void Epoll::manageCgi(Client *client, int byteReads, char *buf)
{
	// std::cout << "MANAGE CGI" << std::endl;
	std::string bufferString(buf, byteReads);
	if (bufferString.empty())
		return ;
	if (client->getResponseClass().getResponseState() == FIRST_READ)
	{	
		size_t pos = bufferString.find("\r");
		if (pos == std::string::npos)
		{
			std::cout << "No found of the new line after header" << std::endl;
			client->setResponseComplete(false);
			return ;
		}
		std::string headerPart = bufferString.substr(0, pos);
		std::string chunk;
		chunk.append("HTTP/1.1 200 OKOK\r\n");
		chunk.append("Transfer-Encoding: chunked\r\n");
		chunk.append(headerPart);
		chunk.append("\r\n\r\n");
		client->getResponseBuffer().push_back(chunk);
		try
		{
			std::string bodyPart = bufferString.substr(pos + 4);		
			formatingchunk(client, bodyPart);
		}
		catch(...)
		{

		}
		client->getResponseClass().setResponseState(NEXT_READ);
		client->setResponseComplete(false);			
	}
	else
	{
		formatingchunk(client, bufferString);
		client->setResponseComplete(false);
	}
}

void Epoll::deleteClient()
{
	if (_client != NULL && _client->getFd() >= 0 )
	{
		std::cout << "Client: "<< _client->getFd()<< " is disconnected" << std::endl;
		epoll_ctl(this->_epFd, EPOLL_CTL_DEL, _client->getFd(), &_ev);
		if (_client->getCgiFd() > 0)
		{
			epoll_ctl(this->_epFd, EPOLL_CTL_DEL, _client->getCgiFd(), &_ev);
			if (_client->getCgiFd() >= 0)
			{
				if (close(_client->getCgiFd()) == -1)
					std::cerr << "Close Function failed (deleteClient cgi)" << std::endl;
			}
		}
		if (close(_client->getFd()) == -1)
			std::cerr << "Close Function failed (deleteClient)" << std::endl;
		_clientsMap.erase(_client->getFd());
		if(_client)
			delete _client;
		_client = NULL;		
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
		if (eventFd == _it->second->getCgiFd())
		{
			_client = _it->second;
			_isCgi = true;
			break;
		}
	}
}

void Epoll::HandleEpollin(int eventFd)
{
	char buf[MAX_READ_SIZE];
	ssize_t byteReads = read(eventFd, buf, sizeof(buf));
	if (byteReads > 0)
	{
		// std::cout << byteReads << std::endl; 
		if (_isCgi == true)
			manageCgi(_client, byteReads, buf);
		else
			manageClientRequest(_clientsMap.at(eventFd), byteReads, buf);
	}
	else if (byteReads == 0)
	{
		if(!_isCgi)
		{
			deleteClient();
		}

	}
	else if (byteReads == -1)
		throw std::runtime_error("Error occurs during the read function (EPOLLIN)\n");
}

void Epoll::HandleEpollout()
{
	if (_client->getResponseBuffer().empty() == 0)
	{
		std::cout << "MESSAGE ENVOYE" << std::endl;
		std::cout << _client->getResponseBuffer().front() << std::endl;
		std::string response = _client->getResponseBuffer().front();
		_client->getResponseBuffer().pop_front();
		ssize_t byteReads = send(_client->getFd(), response.data(), response.size(), 0);
		if (byteReads == -1)
			throw std::runtime_error("Error occurs during the send function (EPOLLOUT)\n");
		if (_client->getResponseBuffer().empty() && _client->getResponseComplete() == true) // a voir mettre secu en plus car fonction send envoie ce qu'il veut 
		{
			std::cout << "send finished" << std::endl;
			std::string connectionType = _client->getRequestClass().getHeader("Connection");			
			if (connectionType == "keep-alive\r")
		  	{  
				_ev.events = EPOLLIN | EPOLLRDHUP |EPOLLERR;
				_ev.data.fd = _client->getFd();            
				epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);
				_client->clearClient();
			}	
			else
			{						
				deleteClient();
			}
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
			try{
				_client->Handle(_client->getRequestClass(), servers[_client->getServerIndex()].getLocations(),  servers[_client->getServerIndex()], _client, *this);	
			}
			catch(const std::exception& e) {
				std::cerr << e.what() << '\n';
				deleteClient();						
			}
			// client->setClientState(SENDING_RESPONSE);				
			_ev.events = EPOLLOUT | EPOLLRDHUP;
			_ev.data.fd = _client->getFd();            
			epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);
		}
	}
}

void Epoll::closeCgiFd()
{
	// std::cout << "DELETE AND CLOSE CGI FD" <<  std::endl;
	epoll_ctl(this->_epFd, EPOLL_CTL_DEL, _client->getCgiFd(), &_ev);    
	_client->getResponseBuffer().push_back("0\r\n\r\n");
	_client->setResponseComplete(true);
	if (_client->getCgiFd() >= 0)
	{
		if (close(_client->getCgiFd()) == -1)
			std::cerr << "Close Function failed in (closeCgiFd)" << std::endl;
		_client->setCgiFd(-1);
	}
}

void Epoll::epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> &servers)
{
	creatEpollFdListeners(listener_fds);
	while (1)
	{
		_eventWait = epoll_wait(_epFd, _events, MAX_CLIENTS, -1);
		// print_ready_events(_eventWait, _events);
		for (int i = 0; i < _eventWait; i++)
		{
			_isCgi = false;
			_is_listener = false;
			_client = NULL;
			try {
				NewClientConnection(listener_fds, _events[i].data.fd);
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << '\n';
			}
			if (_is_listener)
				continue;
			MatchEventWithClient(_events[i].data.fd);
			if (_client == NULL)
				continue;
			if (_events[i].events & EPOLLRDHUP || _events[i].events & EPOLLERR || _events[i].events & EPOLLHUP )
			{
				if (_isCgi)
					closeCgiFd();
				else
					deleteClient();
				continue;
			}
			try {
				if (!_is_listener && (_events[i].events & EPOLLIN))
					HandleEpollin(_events[i].data.fd);
				else if (!_is_listener && (_events[i].events & EPOLLOUT) && _isCgi == false)
					HandleEpollout();
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << '\n';
				deleteClient();
			}
		}
		generatePendingResponse(servers);			
		// printClientMap();		
	}
	return;
}
