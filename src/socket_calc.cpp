/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket_calc.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masase <masase@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/10 19:02:48 by masase            #+#    #+#             */
/*   Updated: 2025/12/10 20:47:45 by masase           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/socket_calc.hpp"

void calculator()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cout << "couldn't create a scoket" << std::endl;
        //exit(EXIT_FAILURE);
    }
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(8080);
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
    {
        std::cout << "couldn't bind" << std::endl;
        //exit(EXIT_FAILURE);        
    }
    if (listen(sockfd, 2) < 0)
    {
        std::cout << "couldn't listen" << std::endl;
        //exit(EXIT_FAILURE);        
    }    
    while (1)
    {
        socklen_t addrlen = sizeof(sockaddr);
        int connection = accept(sockfd, (struct sockaddr*)&sockaddr, &addrlen);
        if (connection < 0)
        {
            std::cout << "couldn't accept" << std::endl;
            //exit(EXIT_FAILURE);                
        }
        char buffer[100];
        size_t byte = read(connection, buffer, 100);
        (void)byte;
        std::cout << "so you wanna know what " <<  buffer << std::endl;
        std::string raiponce = "I'm too tired sorry come back tomoworr ZZZzzzzZZZZzzzzZZZZzzz";
        send(connection, raiponce.c_str(), raiponce.size(), 0);
    }
    // close(connection);
    close(sockfd);
}