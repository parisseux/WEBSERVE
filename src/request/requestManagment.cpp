#include "Request.hpp"
#include "../client/client.hpp"
#include <iomanip>

void printBodyDebug(const std::vector<unsigned char>& body)
{
    std::cout << "Body size = " << body.size() << " bytes\n";

    for (size_t i = 0; i < body.size(); i++)
    {
        std::cout  << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << (int)body[i] << " ";
    }
    std::cout << std::dec << std::endl;
}

void Request::parseBody(Client *client)
{
    std::cout << "PARSE BODY " << std::endl;
    _body = client->getRequestBuffer();
    _bodyBinary.assign(client->getRequestBuffer().begin(), client->getRequestBuffer().end());
    // printBodyDebug(this->_bodyBinary);
    client->setClientState(WAITING);
    client->setReadyToWrite(true);
}
