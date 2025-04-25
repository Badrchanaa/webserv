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
// #include "WebServer.hpp"

struct CGIProcess {
  int client_fd;
  int cgi_sock;

  // int fd_open;

  pid_t pid;
  std::string output;
  std::string input;
  size_t written;
  void handle_output(CGIProcess &proc);
  void handle_input(CGIProcess &proc);
};

class CGIHandler {
private:
  std::map<int, CGIProcess> processes;

public:
  int epoll_fd;
  // CGIHandler(int efd);
  CGIHandler(int ep_fd = -1) : epoll_fd(ep_fd) {}
  // explicit  CGIHandler(int ep_fd = 0) : epoll_fd(ep_fd) {}
  // bool is_cgi_socket(int fd) const;
  int is_cgi_socket(int fd) const;
  int getCgiSocket(int c_fd) const;
  void spawn(const std::string &script, const std::vector<std::string> &env,
             int client, const std::string &body);
  void handle_cgi_request(int fd, uint32_t events);
  void check_zombies();
  void setup_child(int sock, const std::string &script,
                   const std::vector<std::string> &env);
  // read(index.html);
  void cleanup(CGIProcess &proc, bool error);
};

#endif // !__cgi__
