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

#define DEBUG_LOG(msg) std::cout << "[Server] " << msg << std::endl
#define CLIENT_LOG(msg) std::cerr << "[Client] " << msg << std::end

#define MAX_EVENTS 1024
#define BUFFER_SIZE 350
#define BACKLOG 128
#define PORT "1337"

#define EPOLL_ERRORS (EPOLLRDHUP | EPOLLERR | EPOLLHUP)
#define EPOLL_READ (EPOLLIN)
#define EPOLL_WRITE (EPOLLOUT)

class WebServer {

  Config config;
  CGIHandler cgi;
  EpollManager epoll;
  std::list<Connection *> connections;
  volatile bool running;

  std::vector<FileDescriptor *> listener_descriptors;
  std::map<int, std::vector<ConfigServer> >
      listener_map; /* Maps listener FDs to their config*/

public:
  WebServer();
  ~WebServer();
  void run();

  // private:
  void create_listener();

  /* Modified functions */
  void create_listeners();
  void accept_connections(int listen_fd);

  void setup_epoll();
  bool handle_client_request(Connection &conn);
  bool handle_client_response(Connection &conn);
  // void handle_client(int fd, uint32_t events);
  bool handle_client(Connection &conn);
  void log_connection(const struct sockaddr_storage &addr);
  void log_request(const Connection &conn);
  Connection *find_connection(int fd);
  void cleanup_connection(std::list<Connection *>::iterator &it);
  void set_nonblocking(int fd);

  Connection &connection_ref(int fd);
  Connection &getClientConnection(int fd);
  int getCgiFdBasedOnClientFd(int client_fd);
};

#endif // !_test__
