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
	client->setFd(accept(listener_fds.at(j), NULL, NULL));
	Clients_map.insert(std::make_pair(client->getFd(), client));
	int flags = fcntl(Clients_map.at(client->getFd())->getFd(), F_GETFL, 0);
	fcntl(client->getFd(), F_SETFL, flags | O_NONBLOCK);
	_ev.events = EPOLLIN;
	_ev.data.fd = client->getFd();
	epoll_ctl(_ep_fd, EPOLL_CTL_ADD, client->getFd(), &_ev);
	client->getResponseBuffer().clear();
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
void Epoll::manageClientRequest(Client *client, int byteReads, char *buf, std::vector<ServerConfig> servers)
{
	std::cout << "MANAGE CLIENT REQUEST" <<  std::endl;       
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
		if (client->getRequestClass().getMethod() == "POST") 
		{
			// client->getRequestBuffer().append(bufferString);
			// if (client->getContentLength() == client->getRequestBuffer().size())
			// {
				client->getRequestClass().parseRequest(client->getRequestBuffer());     
				client->setClientState(WAITING);
				client->setReadyToWrite(true);                            
				//client->getRequestClass().displayRequest(); // affichage requete complete
				//std::cout << client->getRequestClass().getBody() << std::endl;                
			// }
		}        
		else
		{
			// client->get_requestBuffer().append(bufferString);
		}
	}
	if (client->getReadyToWrite() == true) // client prêt a recevoir une reponse
	{
		client->setClientState(GENERATING_BODY);
		client->getRequestClass().Handle(client->getRequestClass(), servers[0].getLocations(), servers[0], client, *this);
		//Res.displayResponse();
		// client->setResponseBuffer(Res.constructResponse());
		client->setClientState(SENDING_BODY);
		_ev.events = EPOLLOUT ;
		_ev.data.fd = client->getFd();            
		epoll_ctl(this->_ep_fd, EPOLL_CTL_MOD, client->getFd(), &_ev);
	}                            
}

void Epoll::manageCgi(Client *client, int byteReads, char *buf)
{
	std::cout << "MANAGE CGI" << std::endl;
	std::string bufferString(buf, 0, byteReads);
	client->getResponseBuffer().push_back(bufferString);
	if ((bufferString.find("0\r\n\r\n"))!=std::string::npos)
	{
		std::cout << "DELETE AND CLOSE CGI FD" <<  std::endl;
		epoll_ctl(this->_ep_fd, EPOLL_CTL_DEL, client->getCgiFd(), &_ev);    
		close(client->getCgiFd());  
	}
}

void Epoll::epollManagment (std::vector<int>& listener_fds, std::vector<ServerConfig> servers)
{
	creatEpollFdListeners(listener_fds);
	while (1)
	{
		// std::cout << "waiting request..." << std::endl;
		_event_wait = epoll_wait(_ep_fd, _events, 10, -1);
		print_ready_events(_event_wait, _events);
		for (int i = 0; i < _event_wait; i++)
		{
			std::map<int, Client*>::iterator it;
			Client *client;
			Client *client_cgi;
			bool isCgi = false;
			bool is_listener = false;
			for (unsigned int j = 0; j < listener_fds.size(); j++) //cree un nouveau client en cas de nouvelle connexion
			{
				if (_events[i].data.fd == listener_fds.at(j))
				{
					creactNewClient(listener_fds, j);
					is_listener = true;
					break ;
				}
			}
			for (it = Clients_map.begin(); it != Clients_map.end(); ++it) // choisi le bon client en fonction du fd de l'event recu
			{
				if (_events[i].data.fd == it->first)
				{
					client = it->second;
					isCgi = false;
					break ;
				}
				client_cgi = it->second;
				if (_events[i].data.fd == client_cgi->getCgiFd())
				{
					isCgi = true;
					break;
				}
			} 
			if (!is_listener && (_events[i].events & EPOLLOUT) && isCgi == false)
			{
				if (client->getResponseBuffer().empty() == 0 && client->getResponseBuffer().front().empty() == 0)
				{
					std::cout << "MESSAGE ENVOYE" << std::endl;
					std::cout << client->getResponseBuffer().front() << std::endl;
					// writeInAscii(client->getResponseBuffer().front());
					std::string response = client->getResponseBuffer().front();
					client->getResponseBuffer().pop_front();
					std::cout << _events[i].data.fd << client->getFd() << std::endl;
					ssize_t byteReads = send(client->getFd(), response.c_str(), response.size(), 0);
					if (byteReads > 0)
					{
						std::cout << "send some byte" << std::endl;
					}
					if (client->getBodyComplete() == true) // a voir mettre secu en plus car fonction send envoie ce qu'il veut 
					{
						std::cout << "send finished" << std::endl;                        
						_ev.events = EPOLLIN ;
						_ev.data.fd = client->getFd();            
						epoll_ctl(this->_ep_fd, EPOLL_CTL_MOD, client->getFd(), &_ev);
						client->setReadyToWrite(false);
						client->clearRequest();
						close(client->getFd());						
					}
				}
			}
			else if( !is_listener && (_events[i].events & EPOLLIN))
			{
				char buf[4000];
				// std::cout << "FD CGI TROUVE: "<<client_cgi->getCgiFd() << client_cgi->getFd() << std::endl;
				// std::string content;            
				// readFd(client_cgi->getCgiFd(), content);
				// std::cout << content << std::endl;
				// std::cout << client_cgi->getCgiFd() << _events[i].data.fd << std::endl;    
				ssize_t byteReads = read(_events[i].data.fd, buf, sizeof(buf));
				if (byteReads > 0)
				{
					// std::cout << byteReads << std::endl; 
					if (isCgi == true)
					{
						manageCgi(client_cgi, byteReads, buf);
					}
					else
						manageClientRequest(Clients_map.at(_events[i].data.fd), byteReads, buf, servers);
				}
				// buf[0] = '\0';
			
				
			}
		}
	}
	return ;
}
