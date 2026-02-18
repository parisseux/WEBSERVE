#include "Request.hpp"
#include "../client/client.hpp"
#include <iomanip>

void printBodyDebug(const std::vector<unsigned char>& body)
{
    for (size_t i = 0; i < body.size(); i++)
    {
        std::cout   << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << (int)body[i] << " ";
    }
    std::cout << std::dec << std::endl;
    std::cout << "Body size = " << body.size() << " bytes\n";
}

void Request::parseBody(Client *client)
{
    std::cout << "PARSE BODY " << std::endl;
    client->getRequestClass().displayRequest();
    std::cout << client->getContentLength() << std::endl;
    std::cout << "----------------------" << std::endl;
    _body = client->getRequestBuffer();
    _bodyBinary.assign(client->getRequestBuffer().begin(), client->getRequestBuffer().end());
    printBodyDebug(this->_bodyBinary);
    client->setClientState(WAITING);
    client->setReadyToWrite(true);
}
