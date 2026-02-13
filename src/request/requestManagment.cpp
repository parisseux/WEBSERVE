#include "Request.hpp"
#include "../client/client.hpp"
#include <iomanip>

void printBodyDebug(const std::vector<unsigned char>& body)
{
    std::cout << "Body size = " << body.size() << " bytes\n";

    for (size_t i = 0; i < body.size(); i++)
    {
        std::cout // << std::hex
                //   << std::setw(2)
                //   << std::setfill('0')
                  << body[i] << " ";
    }
    // std::cout << std::dec << std::endl;
}

void Request::parseBody(Client *client)
{
    std::cout << "PARSE BODY " << std::endl;
    // if (this->_bodyBinary.size() >= contentLength)
    // {
    //     printBodyDebug(this->_bodyBinary);
    //     return;
    // }
    _body = client->getRequestBuffer();
    _bodyBinary.assign(client->getRequestBuffer().begin(), client->getRequestBuffer().end());
    // std::cout << "BODY BINARY PRINTTTTT" << std::endl;
    // for (unsigned int i = 0; i < _bodyBinary.size() ; ++i)
    // {
    //     std::cout << _bodyBinary[i];
    // }
    // std::cout << std::endl;
    // std::cout << client->getRequestBuffer() << std::endl;
    // printBodyDebug(this->_bodyBinary);
    client->setClientState(WAITING);
    client->setReadyToWrite(true);

    //this->_bodyBinary.insert(_bodyBinary.end(), bufferString.begin(), bufferString.end());
}
