#include "client.hpp"
#include "../response/Response.hpp"
#include "../cgi/cgi.hpp"
#include "../response/StaticTarget.hpp"

// CONSTRUCTOR / DESTRUCTOR

Client::Client()
: _ReadyToWrite(false), _state(WAITING)
{
    std::cout << "Client created" << std::endl;
}

Client::~Client()
{
    std::cout << "Client destructed" << std::endl;
}

// ALL THE SETTESS

void Client::setFd(int fd)
{
    this->_fd = fd;
}

void Client::setFlags(int flags)
{
    this->_flags = flags;
}

void Client::setRequestBuffer(std::string requestBuffer)
{
    this->_requestBuffer = requestBuffer;
}

void Client::setResponseBuffer(std::deque<std::string> responseBuffer)
{
    this->_responseBuffer = responseBuffer;
}

void Client::setReadyToWrite(bool ReadytoWrite)
{
    this->_ReadyToWrite = ReadytoWrite;
}

void Client::setClientState(ClientState state)
{
    this->_state = state;
}

// ALL THE GETTERS

int& Client::getFd()
{
    return (this->_fd);
}

int& Client::getFlags()
{
    return (this->_flags);
}

std::string& Client::getRequestBuffer()
{
    return (this->_requestBuffer);
}

std::deque<std::string>& Client::getResponseBuffer()
{
    return (this->_responseBuffer);
}

bool& Client::getReadyToWrite()
{
    return (this->_ReadyToWrite);
}

Request& Client::getRequestClass()
{
    return (this->_request);
}

ClientState Client::getClientState()
{
    return(this->_state);
}

int Client::getContentLength()
{
    std::string length;
    int content_length;
    char *pEnd;
    length = headerValue("Content-Length", this->getRequestClass());
    content_length = std::strtol(length.c_str(), &pEnd, 0);
	if (*pEnd != '\0')
	{    
		std::cout << "Erreur conversion en int dans calcul content length" << std::endl;              
	}    
    return (content_length);
}

int Client::getResponseBufferLength()
{
    std::deque<std::string>::iterator it;
    int length = 0;
    for (it = _responseBuffer.begin(); it != _responseBuffer.end(); ++it)
    {
        length += it->size();
    }
    return (length);
}

void Client::clearRequest()
{
    getRequestClass().getMethod().clear();
    getRequestClass().getRequestTarget().clear();
    getRequestClass().getPath().clear();
    getRequestClass().getQuery().clear();
    getRequestClass().getProtocol().clear();                     
    getRequestClass().getBody().clear();
    getRequestClass().getHeaders().clear();
}

void Client::clearResponse()
{
    getResponseClass().setStatus(200);
    getResponseClass().getHeaders().clear();
    getResponseClass().setBody("");
    getResponseClass().setResponseState(FIRST_SENT);        
}

void Client::clearClient()
{
    setReadyToWrite(false);
    clearRequest();
    getRequestBuffer().clear();
    clearResponse();							
    getResponseBuffer().clear();
    setClientState(WAITING);
    setBodyComplete(false);
    setByteSentPos(0);
    close(this->getFd());    
}

void Client::Handle(Request &req, const std::vector<LocationConfig>& locations, const ServerConfig &server, Client *client, Epoll &epoll)
{
    // std::cout << "Handling request..." << "of client " << client->getFd() << std::endl;
    int status = req.ValidateRequest(req);
    if (status == 400)
    {
        client->getResponseBuffer().push_front(Response::Error(400, "400 Bad Request").constructResponse());       
        return ; 
    }
    if (status == 501)
    {
        client->getResponseBuffer().push_front(Response::Error(501, "501 Not Implemented").constructResponse());      
        return ;        
    }
    const LocationConfig *loc = req.MatchLocation(req.getPath(), locations);
    if (!loc)
    {
        client->getResponseBuffer().push_front(Response::Error(500, "500 No location matched (unexpected)").constructResponse());    
        return ;          
    }
    status = req.MethodAllowed(req, loc);
    if (status == 405)
    {
        client->getResponseBuffer().push_front(Response::Error(405, "405 Method Not Allowed").constructResponse());       
        return ; 
    }
    // buildRedirectResponse(loc);

    // CGI handler va executer un script ou un process
    if (isCgi(req, server, *loc))
    {
        Cgi cgi;
        cgi.handleCgi(req, server, *loc, client, epoll);
        client->setClientState(GENERATING_CGI);
        return ;
    }
    //upload handler (="POST") va venir écrire dans un fichiers
    // handleUpload(req, server, *loc);

    // static handler va lire un fichier
    //Ici on se charge de trouver la réponse quon doit envoyer au clients
    StaticTarget st;
    ResolvedTarget target = st.ResolveStaticTarget(req, server, *loc);
    st.BuildStaticResponse(req, target, client, _response);
    if(_response.getResponseState() == FIRST_SENT)
    {
        // std::cout << "ON  CREE UNE REPONSE COMPLETE" << std::endl;
        client->getResponseBuffer().push_front(_response.constructResponse().data());
        _response.setResponseState(N_SENT);
    }
    else
    {
        // std::cout << "ON CONTINUE L'ENVOIE" << std::endl;        
        client->getResponseBuffer().push_front(_response.getBody());
    }
    _response.getBody().clear();
}
