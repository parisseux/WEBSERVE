
*This project has been created as part of the 42 curriculum by <your_login>[, <login2>, pchatagn].*

# Webserv

## Description

Webserv is a custom HTTP/1.1 web server written in C++, developed as part of the 42 curriculum.

The goal of this project is to understand how web servers work internally by implementing one from scratch. Instead of using existing solutions such as Nginx or Apache, we recreate the core mechanisms of an HTTP server, including request parsing, response generation, configuration handling, and client connection management.

This project deepens understanding of:
- Network programming (sockets, TCP/IP)
- HTTP protocol (methods, headers, status codes)
- Multiplexing (select, poll, or epoll)
- Process management (CGI handling)
- File handling
- Server architecture and performance considerations

The server is configurable through a configuration file inspired by Nginx syntax.

---

## Features

- HTTP/1.1 support
- Multiple server blocks
- Non-blocking I/O
- GET, POST, DELETE methods
- Static file serving
- Directory listing (autoindex)
- File uploads
- CGI execution (e.g., PHP or Python)
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
    curl -X GET http://localhost:8080  
    curl -X POST -F "file=@test.txt" http://localhost:8080/upload

##Resources 

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
- David ..
- Mawuli 
- Parissa Chatagny

42 School – Webserv Project