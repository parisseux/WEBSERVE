#include "../include/webserv.hpp"

void epoll_managment (const std::vector<int> listener_fd, std::map<int, Client>& Clients)
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
    epoll.ep_fd = epoll_create(10);
    epoll.ev.events = EPOLLIN;
    epoll.ev.data.fd = listener_fd.at(0);
    epoll_ctl(epoll.ep_fd, EPOLL_CTL_ADD, listener_fd.at(0), &epoll.ev);
    while (1)
    {
        epoll.event_wait = epoll_wait(epoll.ep_fd, epoll.events, 10, -1);
        for (int i = 0; i < epoll.event_wait; i++)
        {
            if (epoll.events[i].data.fd == listener_fd.at(0))
            {
                int client_sckt = accept(listener_fd.at(0), nullptr, nullptr);
                int flags = fcntl(client_sckt, F_GETFL, 0);
                fcntl(client_sckt, F_SETFL, flags | O_NONBLOCK);
                epoll.ev.events = EPOLLIN;
                epoll.ev.data.fd = client_sckt;
                epoll_ctl(epoll.ep_fd, EPOLL_CTL_ADD, client_sckt, &epoll.ev);
            }
            else
            {
                if (epoll.events[i].events & EPOLLIN)
                {
                    char buf[1024]; 
                    recv(epoll.events[i].data.fd, buf, sizeof(buf), 0);
                    std::cout << "Requête reçue:\n" << buf << std::endl;
                    // Faut encore fermer les clients
                }
            }
        }
    }
    close(listener_fd.at(0));
    return ;
}
