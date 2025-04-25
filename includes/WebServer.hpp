#ifndef __WebServer__
#define __WebServer__

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream> // Required for file operations
#include <iostream>
#include <list>
#include <map>
#include <netdb.h>
#include <signal.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "Connection.hpp"
#include "HTTPParseState.hpp"
#include "HTTPParser.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

// #include "Connection.hpp"
// #include "EpollManager.hpp"
#include "Config.hpp"
#include "EpollManager.hpp"
#include "FileDescriptor.hpp"

#define DEBUG_LOG(msg) std::cerr << "[Server] " << msg << std::endl
#define CLIENT_LOG(msg) std::cerr << "[Client] " << msg << std::end

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
#define BACKLOG 128
#define PORT "1337"

#define EPOLL_ERRORS (EPOLLRDHUP | EPOLLERR | EPOLLHUP)
#define EPOLL_READ (EPOLLIN)
#define EPOLL_WRITE (EPOLLOUT)

class WebServer {

  FileDescriptor listen_fd; // This is FileDescriptor of the socket
  Config config;
  CGIHandler cgi;
  EpollManager epoll;
  std::list<Connection *> connections;
  volatile bool running;

  std::vector<FileDescriptor> listener_descriptors;
  std::map<int, ServerConfig> listener_map; // Maps listener FDs to their config

public:
  WebServer();
  void run();

  // private:
  void create_listener();
  // void accept_connections();

  /* Modified functions */
  void create_listeners();
  void accept_connections(int listen_fd);

  void setup_epoll();
  void handle_client_request(Connection *conn);
  void handle_client_response(Connection *conn);
  void handle_client(int fd, uint32_t events);
  void log_connection(const struct sockaddr_storage &addr);
  void log_request(const Connection &conn);
  Connection *find_connection(int fd);
  void cleanup_connection(int fd);
  void set_nonblocking(int fd);
};

// In WebServer.hpp
class WebServer {
  // Add these members
  std::vector<FileDescriptor> listener_descriptors;
  std::map<int, ServerConfig> listener_map; // Maps listener FDs to their config

  void create_listeners();
  void accept_connections(int listen_fd);
};

// In WebServer.cpp

void WebServer::run() {
  struct epoll_event events[MAX_EVENTS];

  while (running) {
    int n = epoll_wait(epoll.epfd, events, MAX_EVENTS, -1);

    for (int i = 0; i < n; ++i) {
      int fd = events[i].data.fd;

      // Check if it's a listener socket
      if (listener_map.find(fd) != listener_map.end()) {
        DEBUG_LOG("New connection on listener fd: " << fd);
        accept_connections(fd);
      } else if (cgi.is_cgi_socket(fd)) {
        cgi.handle_cgi_request(fd, events[i].events);
      } else {
        handle_client(fd, events[i].events);
      }
    }
  }
}

void WebServer::accept_connections(int listen_fd) {
  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  ServerConfig &server_conf = listener_map[listen_fd];

  while (true) {
    int new_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (new_fd == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        break;
      throw std::runtime_error("accept failed");
    }

    // Create connection with specific server config
    Connection *conn = new Connection(cgi, server_conf, new_fd);
    connections.push_back(conn);
    epoll.add_fd(new_fd, EPOLLIN | EPOLLRDHUP);
    log_connection(client_addr);
  }
}

// In Connection.hpp
class Connection {
public:
  Connection(CGIHandler &cgihandler, ServerConfig &server_conf, int f)
      : Cgihandler(cgihandler), server_config(server_conf), client_fd(f) {}

  ServerConfig &server_config; // Reference to specific server config
                               // ... rest of the class ...
};

#endif // !_test__
