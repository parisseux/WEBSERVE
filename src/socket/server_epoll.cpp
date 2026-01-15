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

// fonction a call pour gerer EPOLLIN
// static void manage_client_request()
// {
// }

void epoll_managment (std::vector<int>& listener_fds, std::map<int, Client*>& Clients_map)
{
    // int sckt_fd = socket(AF_INET, SOCK_STREAM, 0);
    // int nb_fd;
    // sockaddr_in server_adress;
    // server_adress.sin_family = AF_INET;
    // server_adress.sin_port = htons(8080);
    // server_adress.sin_addr.s_addr = (INADDR_ANY);
    // bind(sckt_fd, (struct sockaddr*) &server_adress, sizeof(server_adress));
    // listen(sckt_fd, MAX_PENDING);
    std::cout << "waiting request..." << std::endl;
    Epoll epoll;
    // epoll.ep_fd = epoll_create(10);
    // epoll.ev.events = EPOLLIN;
    // epoll.ev.data.fd = listener_fds.at(0);
    // epoll_ctl(epoll.ep_fd, EPOLL_CTL_ADD, listener_fds.at(0), &epoll.ev);
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
                char buf[10];
                recv(epoll._events[i].data.fd, buf, sizeof(buf), 0);
                // std::cout << "Requête reçue:\n" << buf << std::endl;                
                std::string bufferString(buf);
                Client* client = Clients_map.at(epoll._events[i].data.fd);

                if (client->get_clientState() == WAITING || client->get_clientState() == READING_HEADER)
                {
                    std::cout << "WE APPEND" << std::endl;
                    client->get_requestBuffer().append(bufferString);
                    client->set_clientState(READING_HEADER);
                }
                if (bufferString.find("\r\n\r\n"))
                {
                    std::cout << "WE FOUND DOUVLE CARRIAGE" << std::endl;                    
                    int found = bufferString.find("\r\n\r\n");                    
                    client->get_requestBuffer().append(bufferString.substr(0, found));                    
                    client->get_requestClass().parseRequest(buf);
                    client->get_requestBuffer().clear();
                    client->get_requestBuffer().append(bufferString.substr(found + 4));
                    client->set_clientState(READING_BODY);
                }

                if (client->get_clientState() == READING_BODY)
                {
                    if (client->get_requestClass().getMethod() == "GET")
                    {
                        client->set_clientState(WAITING);
                        client->set_ReadyToWrite(true);                           
                        Clients_map.at(epoll._events[i].data.fd)->get_requestClass().display_request(); 
                    }
                    client->get_requestBuffer().append(bufferString);              
                }
                if (client->get_ReadyToWrite() == true)
                {
                    //partie parissa qui recoit la recoit la requete complete et peut faire
                    //le routing 
                    //void routing(Clients_map.at((epoll.events[i].data.fd))) // par exemple

                    //reponse basique automatique pour voir que ca marche
                    std::string response = "HTTP/1.0 200 OK\r\n\r\nHELLO";                     
                    write(epoll._events[i].data.fd, response.c_str(), response.size());
                    close(epoll._events[i].data.fd); 
                }              
            }
        }
    }
    // close(listener_fd.at(0));
    return ;
}
