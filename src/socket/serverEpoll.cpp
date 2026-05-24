#include "epoll.hpp"
# include "../response/Response.hpp"
# include "../utils/utils.hpp"
# include <sys/wait.h>
# include <sys/time.h>

volatile int stop = 0;

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
	std::cout << "Client n°" << client->getFd() << " created" << std::endl;
}

void Epoll::HeaderEnd(Client *client)
{
	// std::cout << "FIN DU HEADER" << std::endl;
    unsigned int found = client->getRequestBuffer().find("\r\n\r\n");                                                                                      
    client->getRequestClass().parseRequest(client->getRequestBuffer());
    client->getRequestBuffer().erase(0, found + 4);
    client->setClientState(READING_BODY);
}

// fonction a call pour gerer EPOLLIN
void Epoll::manageClientRequest(Client *client, ssize_t byteReads, char *buf, std::vector<ServerConfig> &servers)
{
	// std::cout << "MANAGE CLIENT " << client->getFd() << " Request " << std::endl; 
    std::string bufferString(buf, byteReads);
    client->getRequestBuffer().append(bufferString);
    if (client->getClientState() == WAITING || client->getClientState() == READING_HEADER)
    {
        size_t pos = client->getRequestBuffer().find("\r\n\r\n");
        bool hasDelimiter = (pos !=std::string::npos); 
        if (hasDelimiter && (client->getClientState() == READING_HEADER || client->getClientState() == WAITING))
            HeaderEnd(client);
		else
		{
			if (client->getRequestBuffer().size() >= 8192 && client->getClientState() == READING_HEADER) // 8KB
			{
				// std::cout << "HEADER TROP GROS" << std::endl;
 				// client->setClientState(GENERATING_RESPONSE);
				client->setResponseComplete(true);
				client->sendError(431, "Request Header Fields Too Large", servers[client->getServerIndex()]);
				_client->setClientState(SENDING_RESPONSE);
				_ev.events = EPOLLOUT | EPOLLRDHUP;
				_ev.data.fd = _client->getFd();            
				epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);					
				return;
			}
			else if (client->getRequestBuffer().size() <= 1)
			{
				// std::cout << "Waiting for header" << std::endl;
				client->setClientState(WAITING_FOR_HEADER);
				return;
			}
			else
			{
				// std::cout << "JE REMET LE MODE READING HEADER" << std::endl;
            	client->setClientState(READING_HEADER);				
			}

		}			
		if (client->getRequestClass().getParseError() != 0)
		{
			client->setRequestComplete(true);
			client->setClientState(GENERATING_RESPONSE);
			return ;
		}
        if (client->getClientState() != READING_BODY)
            client->setClientState(READING_HEADER);
    }
    if (client->getClientState() == READING_BODY)
    {
        if (client->getRequestClass().getMethod() == "GET"
			|| client->getRequestClass().getMethod() == "DELETE")
        {
            client->setRequestComplete(true);
			client->setClientState(GENERATING_RESPONSE);
        }
        if (client->getRequestClass().getMethod() == "POST")
        {
			LocationConfig fallback;
			const LocationConfig *loc = client->getRequestClass().MatchLocation(client->getRequestClass().getPath(), servers[client->getServerIndex()].getLocations(), servers[client->getServerIndex()], fallback);
			if (loc->getHasMaxBodySize())
			{
				if (client->getRequestBuffer().size() > static_cast<unsigned long>(loc->getMaxBodySize()))
				{
					client->setClientState(GENERATING_RESPONSE);
					// std::cout << "STOP DIRECTE TROP GROS" << std::endl;
					client->setResponseComplete(true);
					client->sendError(413, "Payload Too Large", servers[client->getServerIndex()]);
					_client->setClientState(SENDING_RESPONSE);
					_ev.events = EPOLLOUT | EPOLLRDHUP;
					_ev.data.fd = _client->getFd();            
					epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);					
					return;					
				}

    		} 		
            if (client->getRequestBuffer().size() >= client->getContentLength())
            {
                client->getRequestClass().parseBody(client);
				client->setClientState(GENERATING_RESPONSE);

            }
        }
		else
			client->setRequestComplete(true);
    }
    // if (client->getRequestComplete() == true) // client prêt a recevoir une reponse
    // {
	// 	client->setClientState(GENERATING_RESPONSE);
    // }
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
	std::string bufferString(buf, byteReads);
	if (bufferString.empty())
		return ;
	if (client->getResponseClass().getResponseState() == FIRST_READ)
	{	
		size_t pos = bufferString.find("\r");
		if (pos == std::string::npos)
		{
			pos = bufferString.find("\n");
			if (pos == std::string::npos)
			{
				client->setResponseComplete(false);
				return ;
			}
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
		client->setClientState(SENDING_RESPONSE);	
		_ev.events = EPOLLOUT | EPOLLRDHUP;
		_ev.data.fd = _client->getFd();            
		epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);		
	}
	else
	{
		formatingchunk(client, bufferString);
		client->setResponseComplete(false);			
	}
}

void Epoll::deleteClient()
{
	std::cout << "Client n°" << _client->getFd() << " deleted" << std::endl;
	if (_client != NULL && _client->getFd() >= 0 )
	{
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
	_it = _clientsMap.find(eventFd);
	if(_it != _clientsMap.end())
	{
			_client = _it->second;
			_isCgi = false;	
			return ;		
	}
	for (_it = _clientsMap.begin(); _it != _clientsMap.end(); ++_it) // choisi le bon client en fonction du fd de l'event recu
	{
		if (eventFd == _it->second->getCgiFd())
		{
			_client = _it->second;
			_isCgi = true;
			break;
		}
	}
}

void Epoll::HandleEpollin(int eventFd, std::vector<ServerConfig> &servers)
{
	char buf[MAX_READ_SIZE];
	ssize_t byteReads = read(eventFd, buf, sizeof(buf));
	if (byteReads > 0)
	{
		if (_isCgi == true)
			manageCgi(_client, byteReads, buf);
		else if (_client->getClientState() != SENDING_RESPONSE)
			manageClientRequest(_clientsMap.at(eventFd), byteReads, buf, servers);
	}
	else if (byteReads == 0)
	{
		if(!_isCgi)
		{	
            Client *client = _clientsMap.at(eventFd);
            if (!client->getRequestBuffer().empty() &&
                client->getClientState() == READING_HEADER)
            {
                // Reçu des données mais jamais eu \r\n\r\n → 400
                client->sendError(400, "Bad Request", servers[client->getServerIndex()]);
            }					
			deleteClient();
		}

	}
	else if (byteReads == -1)
		throw std::runtime_error("Error occurs during the read function (EPOLLIN)\n");
}

void Epoll::HandleEpollout()
{
	// std::cout << "sending Response" << std::endl;
	if (_client->getResponseBuffer().empty() == 0)
	{
		std::string response = _client->getResponseBuffer().front();
		ssize_t byteSent = send(_client->getFd(), response.data(), response.size(), 0);
		if (response.size() >= 5 && response.substr(0, 5) == "HTTP/")
		{
			size_t end = response.find('\n');
			if (end != std::string::npos)
				std::cout << _client->getRequestClass().getMethod() << " "
						<< _client->getRequestClass().getPath() << " "
						<< response.substr(0, end) << std::endl;
		}

		if (byteSent == -1)
			throw std::runtime_error("Error occurs during the send function (EPOLLOUT)\n");
		_client->addByteSent(byteSent);
		_client->setTimeout(std::time(NULL));
		if (_client->getByteSent() < static_cast<long>(response.size()))
			return;
		_client->getResponseBuffer().pop_front();
		_client->setByteSent(0);
		if (_client->getResponseBuffer().empty() && _client->getResponseComplete() == true)
			deleteClient();
	}
}

void Epoll::generatePendingResponse(std::vector<ServerConfig> &servers)
{
	for (_it = _clientsMap.begin(); _it != _clientsMap.end(); ++_it)
	{
		_client = _it->second;
		if(_client->getClientState() == GENERATING_RESPONSE && _client->getResponseComplete() == false)
		{
			// std::cout << _client->getFd() << " GENERATE RESPONSE" << std::endl;
			try {
				_client->Handle(_client->getRequestClass(), servers[_client->getServerIndex()].getLocations(),  servers[_client->getServerIndex()], _client, *this);	
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << '\n';
				_client->sendError(500, "Internal Server Error", servers[_client->getServerIndex()]);
				_client->setClientState(SENDING_RESPONSE);
				_ev.events = EPOLLOUT | EPOLLRDHUP;
				_ev.data.fd = _client->getFd();            
				epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);			
			}
			if (_client->getResponseBuffer().empty() == 0)
			{
				_ev.events = EPOLLOUT | EPOLLRDHUP;
				_ev.data.fd = _client->getFd();         
				epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);
			}
		}
	}
}

void Epoll::handlingTimeout(std::vector<ServerConfig> &servers)
{
	for (_it = _clientsMap.begin(); _it != _clientsMap.end(); ++_it)
	{
		_client = _it->second;
		// std::cout << "client °" << _client->getFd() << std::endl;
		if (_client->getClientState() == GENERATING_RESPONSE || _client->getClientState() == GENERATING_CGI || _client->getClientState() == WAITING_FOR_HEADER || _client->getClientState() == WAITING || _client->getClientState() == READING_HEADER)
		{
			time_t current_time;
			current_time = std::time(NULL);
			// std::cout << "client °" << _client->getFd() << std::endl;
			if (difftime(current_time, _client->getTimeout()) >= MAX_TIMEOUT)
			{
				if (_client->getClientState() == GENERATING_CGI)
				{
					kill(_client->getCgiPid(), SIGKILL);
				}
				if (_client->getClientState() == GENERATING_CGI)
				{
					kill(_client->getCgiPid(), SIGKILL);
					_client->sendError(504, "Gateway Timeout", servers[_client->getServerIndex()]);
				}
				else
					_client->sendError(504, "Gateway Timeout", servers[_client->getServerIndex()]);
				_client->setClientState(SENDING_RESPONSE);
				_ev.events = EPOLLOUT | EPOLLRDHUP;
				_ev.data.fd = _client->getFd();            
				epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);		
			}
		}
	}
}

void Epoll::closeCgiFd()
{
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

void Epoll::handleCgiAndErrors(std::vector<ServerConfig> &servers)
{
	if (_isCgi)
	{
		int status;
		// std::cout << "waitpid" << std::endl;
		waitpid(_client->getCgiPid(), &status, WNOHANG);
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) > 0)
			{
				// std::cout << "on capte un probleme cgi" << std::endl;
				_client->sendError(500, "Error with the script", servers[_client->getServerIndex()]);
				_client->setClientState(SENDING_RESPONSE);
				_ev.events = EPOLLOUT | EPOLLRDHUP;
				_ev.data.fd = _client->getFd();            
				epoll_ctl(this->_epFd, EPOLL_CTL_MOD, _client->getFd(), &_ev);
				epoll_ctl(this->_epFd, EPOLL_CTL_DEL, _client->getCgiFd(), &_ev);								
			}
			else if (WEXITSTATUS(status) == 0 && _client->getClientState() == SENDING_RESPONSE)
			{							
				std::cout << "on close le CGI" << std::endl;
				closeCgiFd();
			}
		}
	}
	else
		deleteClient();
}
void signalHandler(int sig)
{
    (void)sig;
    stop = 1;
}

void Epoll::epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> &servers)
{
	creatEpollFdListeners(listener_fds);
	while (!stop)
	{
		signal(SIGINT, signalHandler);
		_eventWait = epoll_wait(_epFd, _events, MAX_CLIENTS, 1000);
		// print_ready_events(_eventWait, _events);
		for (int i = 0; i < _eventWait; i++)
		{
			_isCgi = false;
			_is_listener = false;
			_client = NULL;
			try 
			{
				NewClientConnection(listener_fds, _events[i].data.fd);
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
			}
			if (_is_listener)
				continue;
			MatchEventWithClient(_events[i].data.fd);
			if (_client == NULL)
				continue;
			if (_events[i].events & EPOLLRDHUP || _events[i].events & EPOLLERR || _events[i].events & EPOLLHUP )
			{
				handleCgiAndErrors(servers);
				continue;
			}
			try {
				if (!_is_listener && (_events[i].events & EPOLLIN))
					HandleEpollin(_events[i].data.fd, servers);
				else if (!_is_listener && (_events[i].events & EPOLLOUT) && _isCgi == false)
					HandleEpollout();
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << '\n';
				deleteClient();
			}
		}
		handlingTimeout(servers);
		generatePendingResponse(servers);		
	
	}
	std::cout << "Server Off..." << std::endl;
	close(this->_epFd);
	for (size_t i = 0; i < listener_fds.size(); i++)
	{
		close(listener_fds[i]);
	}
	return;
}
