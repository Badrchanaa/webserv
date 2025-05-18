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
#include <signal.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

// CGIHandler

// Response.CGIHandler

// #include ".hpp"
// #include ".hpp"
// #include "WebServer.hpp"
#include "HTTPBody.hpp"

// extern char **environ;
struct CGIProcess {
  CGIProcess(int socket, pid_t p_pid) : cgi_sock(socket), pid(p_pid) {}
  
  int cgi_sock;
  pid_t pid;
  ssize_t read(char *buff, size_t size);
  bool write(HTTPBody &body); // 
  void cleanup(bool error);
};

class CGIHandler {

public:
// private:
  // std::map<int, CGIProcess> processes;
  // explicit  CGIHandler(int ep_fd = 0) : epoll_fd(ep_fd) {}
  // bool is_cgi_socket(int fd) const;
  // CGIHandler(int efd);
  // read(index.html);
  // void cleanup_by_fd(int fd);
  // /
  int epoll_fd;
  CGIHandler(int ep_fd = -1) : epoll_fd(ep_fd) { }
  int is_cgi_socket(int fd) const;
  int getCgiSocket(int c_fd) const;
  CGIProcess *spawn(const std::string &script);
  void handle_cgi_request(int fd, uint32_t events);
  void check_zombies();
  void setup_child(int sock, const std::string &script,
                   char **env);
  void cleanup(const CGIProcess &proc, bool error);
  void  cleanup_by_fd(int fd)
  {
    (void)fd;
  }
};

#endif // !__cgi__
