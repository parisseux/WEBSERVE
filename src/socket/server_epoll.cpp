#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

int main ()
{
    int sckt_fd = socket(AF_INET, SOCK_STREAM, 0);
    int nb_fd;
    sockaddr_in server_adress;
    server_adress.sin_family = AF_INET;
    server_adress.sin_port = htons(8080);
    server_adress.sin_addr.s_addr = (INADDR_ANY);
    bind(sckt_fd, (struct sockaddr*) &server_adress, sizeof(server_adress));
    listen(sckt_fd, 5);
    std::cout << "waiting request..." << std::endl;
    int epfd = epoll_create(10);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sckt_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sckt_fd, &ev);
    struct epoll_event events[10];
    while (1)
    {
        nb_fd = epoll_wait(epfd, events, 10, -1);
        for (int i = 0; i < nb_fd; i++)
        {
            if (events[i].data.fd == sckt_fd)
            {
                int client_sckt = accept(sckt_fd, nullptr, nullptr);
                int flags = fcntl(client_sckt, F_GETFL, 0);
                fcntl(client_sckt, F_SETFL, flags | O_NONBLOCK);
                ev.events = EPOLLIN;
                ev.data.fd = client_sckt;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_sckt, &ev);
            }
            else
            {
                if (events[i].events & EPOLLIN)
                {
                    char buf[1024]; 
                    recv(events[i].data.fd, buf, sizeof(buf), 0);
                    std::cout << "Requête reçue:\n" << buf << std::endl;
                    // Faut encore fermer les clients
                }
            }
        }
    }
    close(sckt_fd);
    return (0);
}
