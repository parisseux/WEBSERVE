FROM ubuntu:jammy
RUN apt update 
RUN apt-get install -y bash
RUN apt-get install -y g++
RUN apt-get install -y make
RUN apt-get install -y git
RUN apt-get install -y nginx
RUN apt-get install -y valgrind
RUN apt-get install -y telnet
RUN apt-get install -y curl
WORKDIR /app
ENTRYPOINT ["/bin/bash"]