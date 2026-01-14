#include "../include/webserv.hpp"

static void creat_epoll_fd_listeners(Epoll& epoll, std::vector<int>& listener_fds)
{
    epoll.ep_fd = epoll_create(10);
    for (int i = 0; i < listener_fds.size(); i++)
    {
        epoll.ev.data.fd = listener_fds.at(i); 
        epoll.ev.events = EPOLLIN;
        epoll_ctl(epoll.ep_fd, EPOLL_CTL_ADD, listener_fds.at(i), &epoll.ev);
    }
}

static void check_new_connexion(Epoll& epoll, std::vector<int>& listener_fds)
{
}

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
        epoll.event_wait = epoll_wait(epoll.ep_fd, epoll.events, 10, -1);
        for (int i = 0; i < epoll.event_wait; i++)
        {
            for (int j = 0; j < listener_fds.size(); j++)
            {
                if (epoll.events[i].data.fd == listener_fds.at(j))
                {
                    Client* client = new Client;
                    client->_fd = accept(listener_fds.at(j), nullptr, nullptr);
                    Clients_map.insert({client->_fd, client});
                    int flags = fcntl(Clients_map.at(client->_fd)->_fd, F_GETFL, 0);
                    fcntl(Clients_map.at(client->_fd)->_fd, F_SETFL, flags | O_NONBLOCK);
                    epoll.ev.events = EPOLLIN;
                    epoll.ev.data.fd = Clients_map.at(client->_fd)->_fd;
                    epoll_ctl(epoll.ep_fd, EPOLL_CTL_ADD, Clients_map.at(client->_fd)->_fd, &epoll.ev);
                }
            }
            if (epoll.events[i].events & EPOLLIN)
            {
                char buf[1024];
                recv(epoll.events[i].data.fd, buf, sizeof(buf), 0);
                std::cout << "Requête reçue:\n" << buf << std::endl;
                // Clients_map.at(epoll.events[i].data.fd)->_requestBuffer.append(buf, std::strlen(buf));
                // Clients_map.at(epoll.events[i].data.fd)->_request.parseRequest(buf);
                // Faut encore fermer les clients
            }
        }
    }
    // close(listener_fd.at(0));
    return ;
}
