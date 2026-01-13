#include "../include/webserv.hpp"

void epoll_managment (int listener_fd, std::map<int, Client*>& Clients_map)
{
    Epoll epoll;
    epoll.ep_fd = epoll_create(10);
    epoll.ev.events = EPOLLIN;
    epoll.ev.data.fd = listener_fd;
    epoll_ctl(epoll.ep_fd, EPOLL_CTL_ADD, listener_fd, &epoll.ev);
    while (1)
    {
        std::cout << "waiting request..." << std::endl;
        epoll.event_wait = epoll_wait(epoll.ep_fd, epoll.events, 10, -1);
        for (int i = 0; i < epoll.event_wait; i++)
        {
            if (epoll.events[i].data.fd == listener_fd)
            {
                Client* client = new Client;
                client->set_fd(accept(listener_fd, NULL, NULL));
                Clients_map.insert(std::make_pair(client->get_fd(), client));
                int flags = fcntl(client->get_fd(), F_GETFL, 0);
                fcntl(client->get_fd(), F_SETFL, flags | O_NONBLOCK);
                epoll.ev.events = EPOLLIN;
                epoll.ev.data.fd = client->get_fd();
                epoll_ctl(epoll.ep_fd, EPOLL_CTL_ADD, client->get_fd(), &epoll.ev);
            }
            else
            {
                if (epoll.events[i].events & EPOLLIN)
                {
                    char buf[1024];
                    recv(epoll.events[i].data.fd, buf, sizeof(buf), 0);
                    // std::cout << "Requête reçue:\n" << buf << std::endl;
                    //envoie de la requete reçue dans le parser
                    Clients_map.at(epoll.events[i].data.fd)->_request.parseRequest(buf);
                    // affichage de la requete parser dans le client
                    Clients_map.at(epoll.events[i].data.fd)->_request.display_request();
                    //set automatique a true en attente de lecteur de requete en continue
                    Clients_map.at(epoll.events[i].data.fd)->set_ReadyToWrite(true);                   
                    if (Clients_map.at((epoll.events[i].data.fd))->get_ReadyToWrite() == true)
                    {
                        //partie parissa qui recoit la recoit la requete complete et peut faire
                        //le routing 
                        //void routing(Clients_map.at((epoll.events[i].data.fd))) // par exemple

                        //reponse basique automatique pour voir que ca marche
                        std::string response = "HTTP/1.0 200 OK\r\n\r\nHELLO";                     
                        write(epoll.events[i].data.fd, response.c_str(), response.size());
                        close(epoll.events[i].data.fd); 
                    }
                }
            }
        }
    }
    // close(listener_fd.at(0));
    return ;
}
