
*This project has been created as part of the 42 curriculum by dchellen, masase, pchatagn.*

# Webserv

## Description

Webserv is a custom HTTP/1.0 web server written in C++, developed as part of the 42 curriculum.

The goal of this project is to understand how web servers work internally by implementing one from scratch. Instead of using existing solutions such as Nginx or Apache, we recreate the core mechanisms of an HTTP server, including request parsing, response generation, configuration handling, and client connection management.

This project deepens understanding of:
- Network programming (sockets, TCP/IP)
- HTTP protocol (methods, headers, status codes)
- Multiplexing (select, poll, or epoll)
- Process management (CGI handling)
- File handling
- Server architecture and performance considerations

The server is configurable through a configuration file inspired by Nginx syntax.

### Basic of an HTTP server

A client (browser, curl) sends an HTTP request, the server reads it, processes it and send back a response. 
Basically the webserv does:
1. opens a socket and listens on a port
2. epoll watches for incoming connections
3. when a client connects, reads the request
4. parse the method, path, header
5. find the matching location in the config
6. server a static file, runs a CGI or handles upload/delete
7. sends the response and close (or keep-alive)

### Epoll

Why Epoll? 
- More efficient than select/poll for many connections — epoll_wait only returns the active fds, while select and poll scan through all monitored fds on every call.

How to use it? 
- epoll_create, epoll_ctl to add/removes fds.
- We use epoll_wait which blocks until one or more fds are ready. It returns only the active fds, so we don't waste time checking idle connections. We use only one epoll_wait. Everything goes through the same loop
  - if the ready fd is the server socket --> we call accept() and add the new client fd to epoll
  - if the ready fd is a client socket --> it's incoming data so we read the request, process it and write the response back. 

What do you monitor?
-  server sockets (new connections) and client sockets (incoming data)

### Methods

- GET: Retrieve a resource from the server. The server finds the requested file and sends it back. No body in the request.
- POST: Send data to the server. We only handle multipart/form-data — the server parses the body, extracts the file, and saves it to the upload directory.
- DELETE: Remove a resource from the server. The server finds the file at the requested path and deletes it. Returns 200 if successful, 404 if the file doesn't exist.

### NGINX

NGINX is a production-grade HTTP server that we used as a reference for our project. 
What we took from NGINX:
- Config structure: server {} blocks with location {} blocks inside
- Directives: listen, root, index, return, client_max_body_size
- Location matching: most specific path wins

Key differences from our webserv:
- NGINX handles thousands of concurrent connections in production — ours is a school project
- NGINX has many more features (reverse proxy, load balancing, SSL, gzip...)
- We only implement a subset of HTTP/1.0 — GET, POST (multipart only), DELETE
- No virtual hosts (out of scope per the subject)

### CGI — Common Gateway Interface

A protocol that allows a web server to execute an external program and send its output as the HTTP response. Instead of serving a static file, the server runs a script and returns what it prints.

Timeout: if the CGI takes too long, the paretn kills the child process and return 504 Gateway Timeout. Otherwise the server would block forver. 

### Important status code

- 2xx : success
- 3xx: redirection
- 4xx: error from client
- 5xx: error from server 

---

## Features

- HTTP/1.0 support
- Multiple server blocks
- Non-blocking I/O
- GET, POST, DELETE methods
- Static file serving
- Directory listing (autoindex)
- File uploads
- CGI execution (.py)
- Custom error pages
- Configurable host and ports

---

## Project Architecture

The server is based on:
- Event-driven architecture
- One main loop handling multiple client connections
- Request parsing system
- Response builder
- Configuration parser
- CGI handler

Main components:
- `Server` – Manages listening sockets
- `Client` – Handles client connection lifecycle
- `Request` – Parses HTTP requests
- `Response` – Generates HTTP responses
- `Config` – Parses and stores configuration file data

---

## Instructions

### Compilation and execution 

To compile the server:
```bash
make
```

To Run the Server:
```bash
./webserv <configuration_file>
```

Since there is no default configuration file, you must specify the path to a configuration file when running the executable.

### Testing
You can test the server using:
- A web browser (Chrome, Firefox, Safari, ...)
- curl

exemple: 
```bash
curl -X GET http://localhost:8080  
```
```bash
curl -X POST -F "file=@test.txt" http://localhost:8080/upload
```
## Resources 

- RFC 7230 & 7231 (protocol HTTP)
- Nginx documentation
- a completer 


## Use of IA
AI tools were used for:
- Clarifying HTTP theory
- Understanding edge cases
- Structuring documentation

No AI-generated code was copied into the project

## Autors
- David Chellen
- Mawuli-Kplim Asase
- Parissa Chatagny

42 School – Webserv Project
