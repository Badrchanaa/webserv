#include "../../includes/WebServer.hpp"
/*

NOTS:

EINTR, indicating that the call was interrupted by a signal handler (page 1339)


AI_PASSIVE : Return socket address structures suitable for a passive open (i.e.,
a listen- ing socket). In this case, host should be NULL, and the IP address
component of the socket address structure(s) returned by result will contain a
wildcard IP address (i.e., INADDR_ANY or IN6ADDR_ANY_INIT)


AI_NUMERICSERV : Interpret service as a numeric port number. This flag prevents
the invoca- tion of any name-resolution service, which is not required if
service is a numeric string


EPOLLET: Employ edge-triggered event notification
EPOLLONESHOT: Disable monitoring after event notification
EPOLLIN: Data other than high-priority data can be read
EPOLLOUT: Normal data can be written
EPOLLERR: An error has occurred
EPOLLHUP: A hangup has occurred
EPOLLRDHUP: Shutdown on peer socket (since Linux 2.6.17)



If no pending connections are present on the queue, and the socket is not
marked as nonblocking, accept() blocks the caller until a connection is present.
If the socket is marked nonblocking and no pending connections are present on
the queue, accept() fails with the error EAGAIN or EWOULDBLOCK.

*/

void WebServer::create_listener() {
  DEBUG_LOG("Initializing network stack...");
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;//AI_PASSIVE | AI_NUMERICSERV;

  DEBUG_LOG("Resolving addresses...");
  int status = getaddrinfo("192.168.122.1", PORT, NULL, &res);
  if (status != 0)
    throw std::runtime_error(gai_strerror(status));

  for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
    char ipstr[INET6_ADDRSTRLEN];
    void *addr;
    int port;
    const char *ipver;

    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      port = ntohs(ipv4->sin_port);
      ipver = "IPv4";
    } else {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      port = ntohs(ipv6->sin6_port);
      ipver = "IPv6";
    }

    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    DEBUG_LOG("Attempting to bind to " << ipver << " " << ipstr << ":" << port);

    /*
    FileDescriptor temp_fd(
        socket(p->ai_family, p->ai_socktype | SOCK_NONBLOCK, p->ai_protocol));
    if (temp_fd.fd == -1) {
      DEBUG_LOG("Socket creation failed: " << strerror(errno));
      continue;
    }

    int yes = 1;
    if (setsockopt(temp_fd.fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      DEBUG_LOG("setsockopt failed: " << strerror(errno));
      continue;
    }

    if (bind(temp_fd.fd, p->ai_addr, p->ai_addrlen) == 0) {
      DEBUG_LOG("Successfully bound to " << ipver << " interface");
      listen_fd.reset(temp_fd.release());
      break;
    } else {
      DEBUG_LOG("Bind failed: " << strerror(errno));
    }
    */
  }

  freeaddrinfo(res);
  if (listen_fd.fd == -1)
    throw std::runtime_error("All bind attempts failed");

  if (listen(listen_fd.fd, BACKLOG) == -1)
    throw std::runtime_error("listen failed");

  DEBUG_LOG("Listening on port " << PORT << " with backlog " << BACKLOG);
}

// void WebServer::setup_epoll() { epoll.add_fd(listen_fd, EPOLLIN); }
void WebServer::setup_epoll() { epoll.add_fd(listen_fd, EPOLLIN | EPOLLET); }

WebServer::WebServer() : running(true) {
  DEBUG_LOG("Initializing web server...");

  create_listener();

  setup_epoll();
  this->cgi.epoll_fd = this->listen_fd;
  DEBUG_LOG("Server initialized. Entering main event loop.");
}

void WebServer::run() {//2000 /500
  
  struct epoll_event events[MAX_EVENTS];
  DEBUG_LOG("Starting main event loop");

  while (running) {
    int n = epoll_wait(epoll.epfd, events, MAX_EVENTS, -1);
    DEBUG_LOG("Epoll_wait returned " << n << " events");

    if (n == -1 && errno != EINTR)
      throw std::runtime_error("epoll_wait failed");

    for (int i = 0; i < n; ++i) {
      if (events[i].data.fd == listen_fd)
      {
        DEBUG_LOG("New connection pending on listening socket");
        this->accept_connections();
      }
      else if (cgi.is_cgi_socket(events[i].data.fd)) 
      {
        this->cgi.handle_cgi_request(events[i].data.fd, events[i].events);
      }
      else
      {
        DEBUG_LOG("Processing event on fd: " << events[i].data.fd);
        this->handle_client(events[i].data.fd, events[i].events);
      }
      // else if (cgi.is_cgi_socket(events[i].data.fd)) 
      // {
      //   if (gettype(event.fd))// RESPONSEWRITE 
      //   {
      //   }
      // }
    }
  }
}

void WebServer::accept_connections() {
  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);

  while (true) { // queue
    int new_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_size);
    if (new_fd == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }
      throw std::runtime_error("accept failed");
    }

    Connection *conn = new Connection(new_fd, client_addr);
    connections.push_back(conn);
    // epoll.add_fd(new_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);
    epoll.subscribe(new_fd, EPOLL_READ, CLIENT_EVENT, &conn->request);

    // epoll.add_fd(new_fd, EPOLLIN | EPOLLRDHUP);

    log_connection(client_addr);
  }
}

// client_fd READ 
void  WebServer::handle_client_request(Connection *conn)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

   bytes_received = recv(conn->fd, buffer, BUFFER_SIZE, 0);
    DEBUG_LOG("Received " << bytes_received << " bytes from fd: " << conn->fd);

    // Print raw received data
    std::cerr << "Raw request data:\n"
              << std::string(buffer, bytes_received)
              << "\n----------------------------------------" << std::endl;


    HTTPRequest &request = conn->request;
    HTTPParser::parse(request, buffer, bytes_received);
    if (request.getParseState().isComplete())
    {
      conn.init_response(request, cgi_process);
      // modify fd for epollout
      this->epoll.mod_fd(conn->fd, EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP);
    }

    if (bytes_received == 0) {
      DEBUG_LOG("Connection closed gracefully (fd: " << fd << ")");
      log_request(*conn);
      cleanup_connection(fd);
    } else if (bytes_received == -1) {
      DEBUG_LOG("Receive error on fd " << fd << ": " << strerror(errno));
      cleanup_connection(fd);
    }
}

// file, cgi READ/WRITE
void  WebServer::handle_client_response(Connection *conn)
{
    conn->response.resume();
    if (conn->response.isComplete)
    {
      response.reset();
      request.reset();
    }

}

void WebServer::handle_client(int fd, uint32_t events) {
  if (events & (EPOLLRDHUP | EPOLLHUP)) {
    DEBUG_LOG("Connection closed by client (fd: "
              << fd << ")" << " -> EPOLLRDHUP || EPOLLHUP Happend!");
    cleanup_connection(fd);
    return;
  }

  Connection *conn = find_connection(fd);
  if (!conn)
    return;
  if (conn->state == Connection::REQUEST && (events & EPOLLIN)) {
      handle_client_request();
  }
}

void WebServer::log_connection(const struct sockaddr_storage &addr) {
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

  DEBUG_LOG("New connection from " << ip << ":" << port);
}

// void WebServer::log_request(const Connection &conn) {
//   size_t end = conn.request.find("\r\n\r\n");
//   if (end != std::string::npos) {
//     std::cout << "HTTP Request:\n"
//               << conn.request.substr(0, end + 4)
//               << "\n----------------------------------------\n";
//   }
// }

// void WebServer::log_request(const Connection &conn) {
//   size_t headers_end = conn.request.find("\r\n\r\n");
//   if (headers_end != std::string::npos) {
//     DEBUG_LOG("Full headers received:");
//     std::cerr << conn.request.substr(0, headers_end + 4)
//               << "\n----------------------------------------" << std::endl;
//   }
//
//   // Log full request if body is complete
//   if (conn.request.length() > headers_end + 4) {
//     DEBUG_LOG("Request body content:");
//     std::cerr << conn.request.substr(headers_end + 4)
//               << "\n----------------------------------------" << std::endl;
//   }
// }

void WebServer::log_request(const Connection &conn) {
  size_t headers_end = conn.request.find("\r\n\r\n");
  if (headers_end != std::string::npos) {
    DEBUG_LOG("Full headers received:");
    std::cerr << conn.request.substr(0, headers_end + 4)
              << "\n----------------------------------------" << std::endl;
  }

  // Check if there's a body to write
  if (conn.request.length() > headers_end + 4) {
    DEBUG_LOG("Request body content:");
    std::string body = conn.request.substr(headers_end + 4);
    std::cerr << body << "\n----------------------------------------"
              << std::endl;

    // Write the body to 'file_output'
    std::ofstream outfile("file_output", std::ios::binary);
    if (outfile) {
      outfile.write(body.c_str(), body.size());
      outfile.close();
    } else {
      std::cerr << "Failed to open 'file_output' for writing." << std::endl;
    }
    if (headers_end != std::string::npos) {
      DEBUG_LOG("Full headers received:");
      std::cerr << conn.request.substr(0, headers_end + 4)
                << "\n----------------------------------------" << std::endl;
      DEBUG_LOG("Full Body Bytes : " << conn.request.size() -
                                            (headers_end + 4));
      std::cerr << "\n----------------------------------------" << std::endl;
    }
  }
}

Connection *WebServer::find_connection(int fd) {
  for (std::list<Connection *>::iterator it = connections.begin();
       it != connections.end(); ++it) {
    if ((*it)->fd == fd)
      return *it;
  }
  return NULL;
}

// void WebServer::cleanup_connection(int fd) {
//   for (std::list<Connection *>::iterator it = connections.begin();
//        it != connections.end(); ++it) {
//     if ((*it)->fd == fd) {
//       delete *it;
//       connections.erase(it);
//       epoll.remove_fd(fd);
//       return;
//     }
//   }
// }

void WebServer::cleanup_connection(int fd) {
  DEBUG_LOG("[Cleanup] Starting cleanup for fd: " << fd);

  for (std::list<Connection *>::iterator it = connections.begin();
       it != connections.end(); ++it) {
    if ((*it)->fd == fd) {
      DEBUG_LOG("[Cleanup] Removing fd " << fd << " from epoll");
      try {
        epoll.remove_fd(fd); // Remove from epoll FIRST
      } catch (const std::exception &e) {
        DEBUG_LOG("[Cleanup] Error removing fd " << fd << ": " << e.what());
      }

      DEBUG_LOG("[Cleanup] Deleting connection object for fd: " << fd);
      delete *it; // This closes the fd via FileDescriptor destructor
      connections.erase(it);

      DEBUG_LOG("[Cleanup] Completed for fd: " << fd);
      return;
    }
  }
  DEBUG_LOG("[Cleanup] FD " << fd << " not found in connections");
}

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
