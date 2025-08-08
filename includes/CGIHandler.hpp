#ifndef __CGIHandler__
#define __CGIHandler__

// #include "./test.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream> // Required for file operations
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <netdb.h>
#include <csignal>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "HTTPBody.hpp"
#include "Constants.hpp"

extern char **environ;

struct CGIProcess {
    RingBuffer<char> m_SocketBuffer;
    int cgi_stdout_sock;
    int cgi_stderr_sock;
    pid_t pid;
    
    CGIProcess(int stdout_sock, int stderr_sock, pid_t p) 
        : m_SocketBuffer(READ_BUFFER_SIZE), cgi_stdout_sock(stdout_sock), cgi_stderr_sock(stderr_sock), pid(p) {}
    
    ssize_t write(HTTPBody &body);
    ssize_t read(char *buff, size_t size);
    ssize_t readStderr(char *buff, size_t size);
    void cleanup(bool error);
};

class CGIHandler {

public:
  int epoll_fd;
  CGIHandler(int ep_fd = -1) : epoll_fd(ep_fd) { }
  int is_cgi_socket(int fd) const;
  int getCgiSocket(int c_fd) const;
  // CGIProcess *spawn(char * const *args) const;
  CGIProcess *spawn(std::string &pathName, std::string &scriptName, char **env) const;
  // CGIProcess *spawn(std::string &pathName, std::string &scriptName, char **env) const;
  void setup_child(int stdout_sock, int stderr_sock, std::string &pathName, std::string &scriptName, char **env) const;
  void handle_cgi_request(int fd, uint32_t events);
  void check_zombies();
  // void setup_child(int sock, char * const *args) const;
  // void setup_child(int sock, std::string &pathName, std::string &scriptName, char **env) const;
};


#endif
