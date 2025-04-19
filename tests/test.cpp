#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
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
    epfd = epoll_create(1337);
    if (epfd == -1)
      throw std::runtime_error("epoll_create1 failed");
  }

  ~EpollManager() {
    if (epfd != -1)
      close(epfd);
  }

  void add_fd(int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
      throw std::runtime_error("epoll_ctl add failed");
  }

  void mod_fd(int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1)
      throw std::runtime_error("epoll_ctl mod failed");
  }

  void remove_fd(int fd) {
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1)
      throw std::runtime_error("epoll_ctl del failed");
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
  FileDescriptor listen_fd;
  EpollManager epoll;
  std::list<Connection *> connections;
  volatile bool running;

public:
  WebServer() : running(true) {
    // setup_signals();
    create_listener();
    setup_epoll();
  }

  void run() {
    struct epoll_event events[MAX_EVENTS];

    while (running) {
      int n = epoll_wait(epoll.epfd, events, MAX_EVENTS, -1);
      if (n == -1 && errno != EINTR)
        throw std::runtime_error("epoll_wait failed");

      for (int i = 0; i < n; ++i) {
        if (events[i].data.fd == listen_fd)
          accept_connections();
        else
          handle_client(events[i].data.fd, events[i].events);
      }
    }
  }

private:
  // void setup_signals() {
  //     struct sigaction sa;
  //     memset(&sa, 0, sizeof(sa));
  //     sa.sa_handler = &WebServer::signal_handler;
  //     sigaction(SIGINT, &sa, NULL);
  //     sigaction(SIGTERM, &sa, NULL);
  // }

  // static void signal_handler(int) {
  //     // Signal handling needs to be static, so we use a global pattern
  //     // Actual shutdown handled in run loop
  //     WebServer::instance().running = false;
  // }

  // void create_listener() {
  //   struct addrinfo hints, *res;
  //   memset(&hints, 0, sizeof hints);
  //   hints.ai_family = AF_UNSPEC;
  //   hints.ai_socktype = SOCK_STREAM;
  //   hints.ai_flags = AI_PASSIVE;
  //
  //   int status = getaddrinfo(NULL, PORT, &hints, &res);
  //   if (status != 0)
  //     throw std::runtime_error(gai_strerror(status));
  //
  //   for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
  //     FileDescriptor temp_fd(
  //         socket(p->ai_family, p->ai_socktype, p->ai_protocol));
  //     if (temp_fd.fd == -1)
  //       continue;
  //
  //     int yes = 1;
  //     if (setsockopt(temp_fd.fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
  //     ==
  //         -1)
  //       continue;
  //
  //     // if (bind(temp_fd.fd, p->ai_addr, p->ai_addrlen) == 0) {
  //     //   listen_fd.reset(temp_fd.release());
  //     //   break;
  //     // }
  //     // Inside the successful bind block:
  //     if (bind(temp_fd.fd, p->ai_addr, p->ai_addrlen) == 0) {
  //       listen_fd.reset(temp_fd.release());
  //
  //       // Add this logging:
  //       if (p->ai_family == AF_INET6) {
  //         std::cout << "Listening on IPv6 address\n";
  //       } else if (p->ai_family == AF_INET) {
  //         std::cout << "Listening on IPv4 address\n";
  //       }
  //       break;
  //     }
  //   }
  //
  //   freeaddrinfo(res);
  //   if (listen_fd.fd == -1)
  //     throw std::runtime_error("bind failed");
  //
  //   if (listen(listen_fd.fd, BACKLOG) == -1)
  //     throw std::runtime_error("listen failed");
  // }

  void create_listener() {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Try both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(NULL, PORT, &hints, &res);
    if (status != 0)
      throw std::runtime_error(gai_strerror(status));

    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
      // Print address info before attempting to bind
      char ipstr[INET6_ADDRSTRLEN];
      void *addr;
      int port;
      const char *ipver;

      // Get pointer to address based on family
      if (p->ai_family == AF_INET) { // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        addr = &(ipv4->sin_addr);
        port = ntohs(ipv4->sin_port);
        ipver = "IPv4";
      } else { // IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
        addr = &(ipv6->sin6_addr);
        port = ntohs(ipv6->sin6_port);
        ipver = "IPv6";
      }

      // Convert IP to string
      inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
      std::cout << "Attempting to bind to " << ipver << " address: " << ipstr
                << ":" << port << std::endl;

      FileDescriptor temp_fd(
          socket(p->ai_family, p->ai_socktype, p->ai_protocol));
      if (temp_fd.fd == -1) {
        std::cerr << "  -> Socket creation failed: " << strerror(errno)
                  << std::endl;
        continue;
      }

      int yes = 1;
      if (setsockopt(temp_fd.fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
          -1) {
        std::cerr << "  -> setsockopt failed: " << strerror(errno) << std::endl;
        continue;
      }

      if (bind(temp_fd.fd, p->ai_addr, p->ai_addrlen) == 0) {
        std::cout << "  -> Successfully bound to " << ipver << " address!"
                  << std::endl;
        listen_fd.reset(temp_fd.release());
        break;
      } else {
        std::cerr << "  -> Bind failed: " << strerror(errno) << std::endl;
      }
    }

    freeaddrinfo(res);
    if (listen_fd.fd == -1)
      throw std::runtime_error("bind failed");

    if (listen(listen_fd.fd, BACKLOG) == -1)
      throw std::runtime_error("listen failed");
  }

  // void create_listener() {
  //     struct addrinfo hints, *res;
  //     memset(&hints, 0, sizeof hints);
  //     hints.ai_family = AF_UNSPEC;
  //     hints.ai_socktype = SOCK_STREAM;
  //     hints.ai_flags = AI_PASSIVE;
  //
  //     int status = getaddrinfo(NULL, PORT, &hints, &res);
  //     if(status != 0) throw std::runtime_error(gai_strerror(status));
  //
  //     for(struct addrinfo* p = res; p != NULL; p = p->ai_next) {
  //         listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
  //         if(listen_fd == -1) continue;
  //
  //         int yes = 1;
  //         setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  //
  //         if(bind(listen_fd, p->ai_addr, p->ai_addrlen) == 0) break;
  //
  //         listen_fd.fd = -1;
  //     }
  //
  //     freeaddrinfo(res);
  //     if(listen_fd == -1) throw std::runtime_error("bind failed");
  //
  //     if(listen(listen_fd, BACKLOG) == -1)
  //         throw std::runtime_error("listen failed");
  // }

  void setup_epoll() { epoll.add_fd(listen_fd, EPOLLIN | EPOLLET); }

  void accept_connections() {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);

    while (true) {
      int new_fd =
          accept(listen_fd, (struct sockaddr *)&client_addr, &addr_size);
      if (new_fd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
          break;
        throw std::runtime_error("accept failed");
      }

      // set_nonblocking(new_fd);
      Connection *conn = new Connection(new_fd, client_addr);
      connections.push_back(conn);
      epoll.add_fd(new_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);

      log_connection(client_addr);
    }
  }

  void handle_client(int fd, uint32_t events) {
    if (events & EPOLLRDHUP || events & EPOLLHUP) {
      cleanup_connection(fd);
      return;
    }

    if (events & EPOLLIN) {
      Connection *conn = find_connection(fd);
      if (!conn)
        return;

      char buffer[BUFFER_SIZE];
      ssize_t bytes_read;

      while ((bytes_read = read(fd, buffer, sizeof(buffer)))) {
        if (bytes_read == -1) {
          if (errno == EAGAIN)
            break;
          cleanup_connection(fd);
          return;
        }

        conn->request.append(buffer, bytes_read);
        if (!conn->headers_complete &&
            conn->request.find("\r\n\r\n") != std::string::npos) {
          conn->headers_complete = true;
          log_request(*conn);
        }
      }
    }
  }

  // void set_nonblocking(int fd) {
  //     int flags = fcntl(fd, F_GETFL, 0);
  //     fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  // }

  void log_connection(const struct sockaddr_storage &addr) {
    char ip[INET6_ADDRSTRLEN];
    int port = 0;

    if (addr.ss_family == AF_INET) {
      struct sockaddr_in *s = (struct sockaddr_in *)&addr;
      inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip));
      port = ntohs(s->sin_port);
    } else {
      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
      inet_ntop(AF_INET6, &s->sin6_addr, ip, sizeof(ip));
      port = ntohs(s->sin6_port);
    }

    std::cout << "New connection from " << ip << ":" << port << std::endl;
  }

  void log_request(const Connection &conn) {
    size_t end = conn.request.find("\r\n\r\n");
    if (end != std::string::npos) {
      std::cout << "HTTP Request:\n"
                << conn.request.substr(0, end + 4)
                << "\n----------------------------------------\n";
    }
  }

  Connection *find_connection(int fd) {
    for (std::list<Connection *>::iterator it = connections.begin();
         it != connections.end(); ++it) {
      if ((*it)->fd == fd)
        return *it;
    }
    return NULL;
  }

  void cleanup_connection(int fd) {
    for (std::list<Connection *>::iterator it = connections.begin();
         it != connections.end(); ++it) {
      if ((*it)->fd == fd) {
        delete *it;
        connections.erase(it);
        epoll.remove_fd(fd);
        return;
      }
    }
  }
};

int main() {
  try {
    WebServer server;
    server.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
