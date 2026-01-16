#include "../include/webserv.hpp"

static void creat_epoll_fd_listeners(Epoll& epoll, std::vector<int>& listener_fds)
{
    epoll._ep_fd = epoll_create(10);
    for (unsigned int i = 0; i < listener_fds.size(); i++)
    {
        epoll._ev.data.fd = listener_fds.at(i); 
        epoll._ev.events = EPOLLIN;
        epoll_ctl(epoll._ep_fd, EPOLL_CTL_ADD, listener_fds.at(i), &epoll._ev);
    }
}

static void creact_new_client(Epoll& epoll, std::vector<int>& listener_fds, std::map<int, Client*>& Clients_map, int j)
{
    Client* client = new Client;
    client->get_fd() = accept(listener_fds.at(j), NULL, NULL);
    Clients_map.insert(std::make_pair(client->get_fd(), client));
    int flags = fcntl(Clients_map.at(client->get_fd())->get_fd(), F_GETFL, 0);
    fcntl(Clients_map.at(client->get_fd())->get_fd(), F_SETFL, flags | O_NONBLOCK);
    epoll._ev.events = EPOLLIN;
    epoll._ev.data.fd = Clients_map.at(client->get_fd())->get_fd();
    epoll_ctl(epoll._ep_fd, EPOLL_CTL_ADD, Clients_map.at(client->get_fd())->get_fd(), &epoll._ev);
}

void HeaderEnd(Client *client, std::string bufferString)
{
    unsigned int found = bufferString.find("\r\n\r\n");                                     
    client->get_requestBuffer().append(bufferString.substr(0, found));                                                         
    client->get_requestClass().parseRequest(client->get_requestBuffer());
    client->get_requestBuffer().clear();
    if (bufferString.size() > found + 4) // a taffer quand on lira le body
    {
        // std::cout << "* on rajoute le reste du buffer au body *" << std::endl;
        // writeInAscii(bufferString);
        // std::cout << bufferString << std::endl;
        client->get_requestBuffer().append(bufferString.substr(found + 4, bufferString.size()));
    }
    client->set_clientState(READING_BODY);
}

// fonction a call pour gerer EPOLLIN
static void manage_client_request(Client *client, int byteReads, char *buf, std::vector<ServerConfig> servers)
{         
    std::string bufferString(buf, 0, byteReads);
    if ((bufferString.find("\r\n\r\n"))!=std::string::npos)
        HeaderEnd(client, bufferString);
    else if (client->get_clientState() == WAITING || client->get_clientState() == READING_HEADER)
    {
        client->get_requestBuffer().append(bufferString);
        client->set_clientState(READING_HEADER);
    }
    if (client->get_clientState() == READING_BODY)
    {
        if (client->get_requestClass().getMethod() == "GET") 
        {
            client->set_clientState(WAITING);
            client->set_ReadyToWrite(true);                 
            // client->get_requestClass().display_request(); // affichage requete complete
        }
        else
        {
            // client->get_requestBuffer().append(bufferString);
        }
    }
    if (client->get_ReadyToWrite() == true) // client prêt a recevoir une reponse
    {
        //partie parissa qui recoit la recoit la requete complete et peut faire routing reponse
        Response Res = HandleRequest(client->get_requestClass(), servers[0].locations, servers[0]);
        // Res.display_response(); 
        std::string responseString = Res.constructResponse();
        // std::cout << "string response" << std::endl;
        // std::cout << responseString << std::endl;   

        //reponse basique automatique pour voir que ca marche
        // std::string response = "HTTP/1.0 200 OK\r\n\r\nHELLO";
        write(client->get_fd(), responseString.c_str(), responseString.size());
        client->set_ReadyToWrite(false);
        client->clearRequest();   
        close(client->get_fd()); 
    }                            
}

void epoll_managment (std::vector<int>& listener_fds, std::map<int, Client*>& Clients_map, std::vector<ServerConfig> servers)
{
    // std::cout << "waiting request..." << std::endl;
    Epoll epoll;
    creat_epoll_fd_listeners(epoll, listener_fds);
    while (1)
    {
        std::cout << "waiting request..." << std::endl;
        epoll._event_wait = epoll_wait(epoll._ep_fd, epoll._events, 10, -1);
        for (int i = 0; i < epoll._event_wait; i++)
        {
            bool is_listener = false;
            for (unsigned int j = 0; j < listener_fds.size(); j++)
            {
                if (epoll._events[i].data.fd == listener_fds.at(j))
                {
                    creact_new_client(epoll, listener_fds, Clients_map, j);
                    is_listener = true;
                    break;
                }
            }
            if (!is_listener && (epoll._events[i].events & EPOLLIN))
            {
                char buf[1024];
                size_t byteReads = recv(epoll._events[i].data.fd, buf, sizeof(buf), 0);
                if (byteReads > 0)
                    manage_client_request(Clients_map.at(epoll._events[i].data.fd), byteReads, buf, servers);
            }
        }
    }
    return ;
}
