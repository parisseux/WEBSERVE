#include "Request.hpp"
#include "../client/client.hpp"

void Request::parseBody(Client* client, std::string& bufferString)
{
    if (this->_bodyBinary.size() == client->getContentLength())
    this->_bodyBinary.insert(_bodyBinary.end(), bufferString.begin(), bufferString.end());
}
