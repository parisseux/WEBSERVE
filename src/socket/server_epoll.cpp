#include "../include/webserv.hpp"

static void creat_epoll_fd_listeners(Epoll& epoll, std::vector<int>& listener_fds)
{
    epoll._ep_fd = epoll_create(10);
    for (int i = 0; i < listener_fds.size(); i++)
    {
        epoll._ev.data.fd = listener_fds.at(i); 
        epoll._ev.events = EPOLLIN;
        epoll_ctl(epoll._ep_fd, EPOLL_CTL_ADD, listener_fds.at(i), &epoll._ev);
    }
}

static void creact_new_client(Epoll& epoll, std::vector<int>& listener_fds, std::map<int, Client*>& Clients_map, int j)
{
    Client* client = new Client;
    client->get_fd() = accept(listener_fds.at(j), nullptr, nullptr);
    Clients_map.insert({client->get_fd(), client});
    int flags = fcntl(Clients_map.at(client->get_fd())->get_fd(), F_GETFL, 0);
    fcntl(Clients_map.at(client->get_fd())->get_fd(), F_SETFL, flags | O_NONBLOCK);
    epoll._ev.events = EPOLLIN;
    epoll._ev.data.fd = Clients_map.at(client->get_fd())->get_fd();
    epoll_ctl(epoll._ep_fd, EPOLL_CTL_ADD, Clients_map.at(client->get_fd())->get_fd(), &epoll._ev);
}

// fonction a call pour gerer EPOLLIN
static void manage_client_request()
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
        std::cout << "waiting request..." << std::endl;
        epoll._event_wait = epoll_wait(epoll._ep_fd, epoll._events, 10, -1);
        for (int i = 0; i < epoll._event_wait; i++)
        {
            bool is_listener = false;
            for (int j = 0; j < listener_fds.size(); j++)
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
                recv(epoll._events[i].data.fd, buf, sizeof(buf), 0);
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
