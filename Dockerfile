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
RUN apt-get install -y python3
RUN apt-get install -y gdb
WORKDIR /app
ENTRYPOINT ["/bin/bash"]