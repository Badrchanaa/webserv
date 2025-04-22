#ifndef __test__
#define __test__


befor resume:
///// open -==> fd
ev.fd= -1
///// this->mod_fd(epfd , MOD, client, &ev )


///



#include "./cgi.hpp"
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

#include "http/HTTPRequest.hpp"
#include "http/HTTPParser.hpp"
#include "http/HTTPParseState.hpp"

#include "http/HTTPResponse.hpp"

#define DEBUG_LOG(msg) std::cerr << "[Server] " << msg << std::endl
#define CLIENT_LOG(msg) std::cerr << "[Client] " << msg << std::end

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
#define BACKLOG 128
#define PORT "1337"

#define   EPOLL_ERRORS (EPOLLRDHUP | EPOLLERR | EPOLLHUP)

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

class Event
{
  public:
    int fd;
    int flags;
    int type;
    void *listener;
};

class EpollManager {
  std::map<int, Event> events;
// public:
//   typedef enum
//   {
//     REQUEST_READ,
//     REQUEST_WRITE,
//     RESPONSE_READ,
//     RESPONSE_WRITE
//   } ;
public:
  int epfd;
// jjjjjjjj
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
    DEBUG_LOG("[Epoll] Adding fd "
              << fd << " with events: " << format_events(events));
    struct epoll_event ev;
    ev.events = events | EPOLL_ERRORS;
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      DEBUG_LOG("[Epoll] Add failed (fd: " << fd << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl add failed");
    }
    DEBUG_LOG("[Epoll] Successfully added fd " << fd);
  }

  void mod_fd(int fd, uint32_t events) {
    DEBUG_LOG("[Epoll] Modifying fd "
              << fd << " with events: " << format_events(events));
    struct epoll_event ev;
    ev.events = events | EPOLL_ERRORS;
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
      DEBUG_LOG("[Epoll] Modify failed (fd: " << fd
                                              << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl mod failed");
    }
    DEBUG_LOG("[Epoll] Successfully modified fd " << fd);
  }

  void remove_fd(int fd) {
    DEBUG_LOG("[Epoll] Attempting to remove fd: " << fd);
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
      DEBUG_LOG("[Epoll] Remove failed (fd: " << fd
                                              << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl del failed");
    }
    DEBUG_LOG("[Epoll] Successfully removed fd: " << fd);
  }

  int subscribe(int fd, int flags, int type, void *listener)
  {
    // Event event;
    // event.listener = listener;
    std::map<int, Event>::iterator it = events.find(fd);
    if (it != events.end())
      return 1;

    events[fd] = {
      fd,
      flags,
      type,
      listener
    };
    this->add_fd(fd, flags | EPOLL_ERRORS);
    this->add_mod(fd, flags | EPOLL_ERRORS);

  }

  void  wait()
  {
    epoll_wait();
    for (event in epoll_events)
    {
      if (events.(event.fd))
      {
        notify(event);
      }
    }
  }

  void unregister(int fd)
  {
    // delete from map and epoll context
    
  }

  void notify(Event &event)
  {
    if (event.type == REQUEST)
    {
      HTTPRequest *request = static_cast<HTTPRequest *>(event.listener);
      bool should_delete = request->resume(event.fd);
    }
    else
    {
      HTTPResponse *response = static_cast<HTTPResponse *>(event.listener);
      bool should_delete = response->resume(event.fd);
    }
    if (should_delete)
    {
      epoll_ctl DELETE;
    }
    // event.listener
  }

private:
  std::string format_events(uint32_t events) {
    std::string result;
    if (events & EPOLLIN)
      result += "EPOLLIN|";
    if (events & EPOLLOUT)
      result += "EPOLLOUT|";
    if (events & EPOLLRDHUP)
      result += "EPOLLRDHUP|";
    if (events & EPOLLET)
      result += "EPOLLET|";
    if (events & EPOLLONESHOT)
      result += "EPOLLONESHOT|";
    if (events & EPOLLERR)
      result += "EPOLLERR|";
    if (events & EPOLLHUP)
      result += "EPOLLHUP|";

    return result.empty() ? "NONE" : result;
  }
};

class Connection {
public:
  typedef enum 
  {
    REQUEST,
    RESPONSE
  } connectionState;
public:
  FileDescriptor fd;
  //std::string request;
  HTTPRequest request;
  HTTPResponse  response;
  connectionState state;
  void  init_response(HTTPRequest &request)
  {
    this->state = RESPONSE;
    this->response.init(request, handler);
  }
  struct sockaddr_storage addr;

  Connection(int f, struct sockaddr_storage a)
      : fd(f), state(REQUEST), addr(a) {}
};

class WebServer {

  FileDescriptor listen_fd; // This is FileDescriptor of the socket
  CGIHandler cgi;
  EpollManager epoll;
  std::list<Connection *> connections;
  volatile bool running;

public:
  WebServer();
  void run();

  // private:
  void create_listener();
  void setup_epoll();
  void accept_connections();
  void  handle_client_request(Connection *conn);
  void  handle_client_response(Connection *conn);
  void handle_client(int fd, uint32_t events);
  void log_connection(const struct sockaddr_storage &addr);
  void log_request(const Connection &conn);
  Connection *find_connection(int fd);
  void cleanup_connection(int fd);
  void set_nonblocking(int fd);
};

#endif // !_test__
