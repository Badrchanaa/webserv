#ifndef __WebServer__
#define __WebServer__

#include <ctime>
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
#include <csignal>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// #include "Connection.hpp"
#include "HTTPParseState.hpp"
#include "HTTPParser.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

// #include "Connection.hpp"
// #include "EpollManager.hpp"
#include "Config.hpp"
#include "EpollManager.hpp"
#include "FileDescriptor.hpp"

// #define DEBUG_LOG(msg) std::cout << "[Server] " << msg << std::endl
#define DEBUG_LOG(msg) ((void)10)
#define CLIENT_LOG(msg) std::cerr << "[Client] " << msg << std::end

#define MAX_EVENTS 1024
#define BUFFER_SIZE 65536
#define BACKLOG 128

// #define EPOLL_ERRORS (EPOLLRDHUP | EPOLLERR | EPOLLHUP)
#define EPOLL_ERRORS (0)
#define EPOLL_READ (EPOLLIN)
#define EPOLL_WRITE (EPOLLOUT)

#define TIMEOUT_SEC 5



class Connection;

class WebServer
{

private:
  WebServer();
  Config config;
  CGIHandler cgi;
  EpollManager epoll;
  std::list<Connection *> connections;
  volatile bool running;

  std::vector<FileDescriptor *> listener_descriptors;
  std::map<int, std::vector<ConfigServer> >
      listener_map; /* Maps listener FDs to their config*/

  static WebServer* instance;

public:
  WebServer(const char *FileCofig);
  ~WebServer();
  void run();

  // private:
  void create_listener();

  void close_fds(Connection &connection);
  /* Modified functions */
  void create_listeners();
  void accept_connections(int listen_fd);

  void setup_epoll();
  void handle_connection_timeout(std::list<Connection *>::iterator &it);
  bool handle_client_request(Connection &conn);
  bool handle_client_response(Connection &conn);
  // void handle_client(int fd, uint32_t events);
  bool handle_client(Connection &conn);
  void log_connection(const struct sockaddr_storage &addr);
  void log_request(const Connection &conn);
  Connection *find_connection(int fd);
  void cleanup_connection(std::list<Connection *>::iterator &it);
  // void cleanup_connection(Connection *conn);
  void set_nonblocking(int fd);

  Connection &connection_ref(int fd);
  Connection &getClientConnection(int fd, uint32_t events);
  int getCgiFdBasedOnClientFd(int client_fd);
  bool try_attach_to_existing_listener(const ConfigServer& new_server, int port);
  // static int  sig_interrupt_handler();

  static void sig_interrupt_handler(int signum);
  void setupSignalHandler();
  void handle_sigint(int signum);
};

#endif // !_test__
