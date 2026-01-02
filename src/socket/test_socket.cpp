#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main ()
{
    int sckt_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_adress;
    server_adress.sin_family = AF_INET;
    server_adress.sin_port = htons(8080);
    server_adress.sin_addr.s_addr = (INADDR_ANY);
    bind(sckt_fd, (struct sockaddr*) &server_adress, sizeof(server_adress));
    listen(sckt_fd, 5);
    std::cout << "waiting request..." << std::endl;
    while (1)
    {
        int client_sckt = accept(sckt_fd, nullptr, nullptr);
        char buf[1024];
        recv(client_sckt, buf, sizeof(buf), 0);
        std::cout << "Requête reçue:\n" << buf << std::endl;
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 15\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Server Respond\n";
        send(client_sckt, response, std::strlen(response), 0);
        close(client_sckt);
    }
    close(sckt_fd);
    return (0);
}
