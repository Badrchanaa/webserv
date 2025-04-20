#ifndef __test__
#define __test__

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <fstream> // Required for file operations
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
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


#define DEBUG_LOG(msg) std::cerr << "[Server] " << msg << std::endl
#define CLIENT_LOG(msg) std::cerr << "[Client] " << msg << std::end

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
#define BACKLOG 128
#define PORT "1337"

class FileDescriptor {
public:
  int fd;

  explicit FileDescriptor(int f = -1) : fd(f) {}
  ~FileDescriptor() { reset(); }

  void reset(int new_fd = -1) {
    if (fd != -1)
      close(fd);
    fd = new_fd;
  }

  int release() {
    int old_fd = fd;
    fd = -1;
    return old_fd;
  }

  operator int() const { return fd; }

private:
  FileDescriptor(const FileDescriptor &);
  FileDescriptor &operator=(const FileDescriptor &);
};

class EpollManager {
public:
  int epfd;

  EpollManager() {
    DEBUG_LOG("[Epoll] Creating epoll instance");
    epfd = epoll_create(1337);
    if (epfd == -1) {
      DEBUG_LOG("[Epoll] Creation failed: " << strerror(errno));
      throw std::runtime_error("epoll_create failed");
    }
    DEBUG_LOG("[Epoll] Created successfully (fd: " << epfd << ")");
  }

  ~EpollManager() {
    DEBUG_LOG("[Epoll] Destroying epoll instance (fd: " << epfd << ")");
    if (epfd != -1) {
      if (close(epfd)) {
        DEBUG_LOG("[Epoll] Close failed: " << strerror(errno));
      }
    }
  }

  void add_fd(int fd, uint32_t events) {
    DEBUG_LOG("[Epoll] Adding fd " << fd << " with events: " << format_events(events));
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      DEBUG_LOG("[Epoll] Add failed (fd: " << fd << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl add failed");
    }
    DEBUG_LOG("[Epoll] Successfully added fd " << fd);
  }

  void mod_fd(int fd, uint32_t events) {
    DEBUG_LOG("[Epoll] Modifying fd " << fd << " with events: " << format_events(events));
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
      DEBUG_LOG("[Epoll] Modify failed (fd: " << fd << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl mod failed");
    }
    DEBUG_LOG("[Epoll] Successfully modified fd " << fd);
  }

  void remove_fd(int fd) {
    DEBUG_LOG("[Epoll] Removing fd " << fd);
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
      DEBUG_LOG("[Epoll] Remove failed (fd: " << fd << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl del failed");
    }
    DEBUG_LOG("[Epoll] Successfully removed fd " << fd);
  }

private:
  std::string format_events(uint32_t events) {
    std::string result;
    if (events & EPOLLIN) result += "EPOLLIN|";
    if (events & EPOLLOUT) result += "EPOLLOUT|";
    if (events & EPOLLRDHUP) result += "EPOLLRDHUP|";
    if (events & EPOLLET) result += "EPOLLET|";
    if (events & EPOLLONESHOT) result += "EPOLLONESHOT|";
    if (events & EPOLLERR) result += "EPOLLERR|";
    if (events & EPOLLHUP) result += "EPOLLHUP|";
    
    return result.empty() ? "NONE" : result;
  }
};

class Connection {
public:
  FileDescriptor fd;
  std::string request;
  bool headers_complete;
  struct sockaddr_storage addr;

  Connection(int f, struct sockaddr_storage a)
      : fd(f), headers_complete(false), addr(a) {}
};

class WebServer {

  FileDescriptor listen_fd; // This is FileDescriptor of the socket
  EpollManager epoll;
  std::list<Connection *> connections;
  volatile bool running;

public:
  WebServer();
  void run();

private:
  void create_listener();
  void setup_epoll();
  void accept_connections();
  void handle_client(int fd, uint32_t events);
  void log_connection(const struct sockaddr_storage &addr);
  void log_request(const Connection &conn);
  Connection *find_connection(int fd);
  void cleanup_connection(int fd);
  void set_nonblocking(int fd);
};

#endif // !_test__
