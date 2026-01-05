
#include "webserv.hpp"

request basic_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // creation de la socket du server
    if (sockfd == -1)
    {
        std::cout << "couldn't create a scoket" << std::endl;
        exit(EXIT_FAILURE);
    }
    // setup socket adress parameters 
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(8080);
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) // on attache les parametre setup a la socket 
    {
        std::cout << "couldn't bind" << std::endl;
        exit(EXIT_FAILURE);       
    }
    if (listen(sockfd, 100) < 0) // la socket peut mtn accpeter les connections avec une queue d'attente de 5 maximum
    {
        std::cout << "couldn't listen" << std::endl;
        exit(EXIT_FAILURE);        
    }    
    while (1)
    {
        socklen_t addrlen = sizeof(sockaddr); // avoir la taille de la struct de l'adresse de la socket

        std::cout << "en attente d'une connection..." <<  std::endl;
        int clientfd = accept(sockfd, (struct sockaddr*)&sockaddr, &addrlen); //  on attent une connection et on lui donne un fd
        if (clientfd < 0)
        {
            std::cout << "couldn't accept" << std::endl;
            //exit(EXIT_FAILURE);                
        }
        char buffer[MAXBYTE - 1];
        int read_byte;
        std::string total_request;
        while ((read_byte = read(clientfd, buffer, MAXBYTE - 1)) > 0) // on lit le message de request
        {
            total_request += buffer;
            if (buffer[read_byte - 1] == '\n')
            {
                break ;
            }
        }
        // std::cout << total_request << std::endl;
        request current_request(total_request);
        // current_request.display_request();
        if (read_byte < 0)
            std::cout << "Error with read loop" << std::endl;
        std::string response = "HTTP/1.0 200 OK\r\n\r\nHELLO"; //message de reponse basqiue
        // std::string response = // response qui permet une request POST avec un body (pas completement fonctionnel)
        // "HTTP/1.0 200 OK\r\n"
        // "Content-Type: text/html\r\n\r\n"
        // "<!DOCTYPE html>"
        // "<html>"
        // "<body>"
        // "<form action=\"/example\" method=\"POST\">"
        // "<input name=\"username\" value=\"alex\">"
        // "<button type=\"submit\">Send POST</button>"
        // "</form>"
        // "<script>"
        // "alert(\"Hello from Java\");"
        // "</script>"
        // "</body>"
        // "</html>";              
        write(clientfd, response.c_str(), response.size());
        close(clientfd);
        return (current_request);        
    }
    close(sockfd);

}