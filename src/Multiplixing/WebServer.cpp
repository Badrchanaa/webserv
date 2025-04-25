#include "../../includes/WebServer.hpp"
#include "../../includes/Connection.hpp"

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
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  DEBUG_LOG("Resolving addresses...");
  int status = getaddrinfo(NULL, PORT, &hints, &res);
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
  }

  freeaddrinfo(res);
  if (listen_fd.fd == -1)
    throw std::runtime_error("All bind attempts failed");

  if (listen(listen_fd.fd, BACKLOG) == -1)
    throw std::runtime_error("listen failed");

  DEBUG_LOG("Listening on port " << PORT << " with backlog " << BACKLOG);
}

void WebServer::setup_epoll() { epoll.add_fd(listen_fd, EPOLLIN); }
// void WebServer::setup_epoll() { epoll.add_fd(listen_fd, EPOLLIN | EPOLLET); }

WebServer::WebServer() : running(true) {
  DEBUG_LOG("Initializing web server...");

  create_listener();

  setup_epoll();
  this->cgi.epoll_fd = this->listen_fd;
  DEBUG_LOG("Server initialized. Entering main event loop.");
}

Connection &WebServer::connection_ref(int fd) {
  for (std::list<Connection *>::iterator it = connections.begin();
       it != connections.end(); ++it) {
    if ((*it)->client_fd == fd)
      return *(*it);
  }
  throw std::runtime_error("Clinet Connection not found");
}

Connection &WebServer::getClientConnection(int fd) {
  try {
    Connection &conn = this->connection_ref(fd);
    conn.socketEvent = true;
    return conn;
  } catch (const std::exception &e) {
    int client_fd = this->cgi.is_cgi_socket(fd);
    if (client_fd) {
      Connection &conn = this->connection_ref(fd);
      conn.cgiEvent = true;
      return conn;
    }
  }
  throw std::runtime_error("CGI Connection not found");
}

void WebServer::run() {
  struct epoll_event events[MAX_EVENTS];
  DEBUG_LOG("Starting main event loop");

  while (running) {
    int n = epoll_wait(epoll.epfd, events, MAX_EVENTS, -1);
    DEBUG_LOG("Epoll_wait returned " << n << " events");

    if (n == -1 && errno != EINTR)
      throw std::runtime_error("epoll_wait failed");

    for (int i = 0; i < n; ++i) {
      if (events[i].data.fd == listen_fd) {
        DEBUG_LOG("New connection pending on listening socket");
        this->accept_connections();
        // } else if (cgi.is_cgi_socket(events[i].data.fd)) {
        //   this->cgi.handle_cgi_request(events[i].data.fd,
        //   events[i].events);
      } else {

        DEBUG_LOG("Processing event on fd: " << events[i].data.fd);
        Connection &conn = this->getClientConnection(events[i].data.fd);
        conn.hasEvent = true;
        conn.events = events[i].events;
        // this->handle_client(events[i].data.fd, events[i].events);
      }
    }
    for (std::list<Connection *>::iterator it = this->connections.begin();
         it != connections.end(); ++it) {
      if ((*it)->hasEvent) {
        this->handle_client(*(*it));
      }
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

    Connection *conn = new Connection(this->cgi, this->config, new_fd, false);
    connections.push_back(conn);
    epoll.add_fd(new_fd, EPOLL_READ);
    // epoll.add_fd(new_fd, EPOLLIN | EPOLLRDHUP);

    log_connection(client_addr);
  }
}

// void  WebServer::handle_client_response(Connection *connection)
// {
//   HTTPResponse  &response = conn->response;
//
//       /*
//       if cgi connection should add socketpair to epollmanager
//       else response is immediate
//       */
//
//   response.resume(false, false);
//   if (response.isDone()) // response is sent
//   {
//     if (!response.isKeepAlive())
//     {
//       // delete connection
//       // connection->m_State = Connection::REQUEST_PARSING;
//       return cleanup_connection(connection->client_fd);
//     }
//     connection->m_State = Connection::REQUEST_PARSING;
//     //reset request
//
//   }
//   else
//   {
//     state = connection.m_Response.getState();
//     if (state == CGI_WRITE)
//     {
//       // remove client_fd from instans monitoring for best perfomance
//
//     }
//     if (state == CGI_READ)
//     {
//       // remove client_fd from instans monitoring for best perfomance
//
//     }
//     if (state == SOCKET_WRITE)
//     {
//       // remove cgi from instans monitoring for best perfomance
//
//     }
//       conn->m_response.getState(); // Done and soket check keep-alive and
//       remove cgi form instans on kernal
//       // CGI Write remove socket_fd from epoll, add cgi socket to epoll
//       // CGI read -
//   }
// }

void WebServer::handle_client_response(int fd, Connection *conn) {
  HTTPResponse &response = conn->m_Response;
  /// sadfdsafdsfdsa
  bool is_cgi = cgi.is_cgi_socket(fd);
  response.resume(false, is_cgi);

  if (response.isDone()) {
    if (!response.isKeepAlive()) {
      cleanup_connection(conn->client_fd);
    } else {
      conn->m_State = Connection::REQUEST_PARSING;
      /// nots this///
      epoll.mod_fd(conn->client_fd, EPOLL_READ | EPOLL_WRITE);
      conn->reset();
    }
  } else {
    int state = conn->m_Response.getState();
    int cgi_sock = conn->Cgihandler.getCgiSocket(conn->client_fd);

    if (state == HTTPResponse::CGI_WRITE) {
      // Switch monitoring to CGI socket for writing
      epoll.remove_fd(conn->client_fd);
      epoll.add_fd(cgi_sock, EPOLL_WRITE | EPOLL_READ);
      DEBUG_LOG("Switched to monitoring CGI socket (write)");

    } else if (state == HTTPResponse::CGI_READ) {
      // Switch monitoring to CGI socket for reading
      epoll.remove_fd(conn->client_fd);
      epoll.add_fd(cgi_sock, EPOLL_WRITE | EPOLL_READ);
      DEBUG_LOG("Switched to monitoring CGI socket (read)");

    } else if (state == HTTPResponse::SOCKET_WRITE) {
      // Switch back to client socket for writing
      if (cgi_sock != -1) {
        epoll.remove_fd(cgi_sock);
        // response.clearCgiSocket();
      }
      epoll.mod_fd(conn->client_fd, EPOLL_READ | EPOLL_WRITE);
    }
  }
}

// void HTTPResponse::clearCgiSocket() {
//   cgi_sock = -1;
//   cgi_state = NONE;
// }

void WebServer::handle_client_request(Connection *connection) {
  char buffer[BUFFER_SIZE];
  ssize_t bytes_received;

  bytes_received = recv(connection->client_fd, buffer, sizeof(buffer), 0);
  if (bytes_received == 0) {
    // Should send response ??
    connection->init_response();
    handle_client_response(connection);
    DEBUG_LOG("Connection closed by client (fd: " << connection->client_fd
                                                  << ")");
    cleanup_connection(connection->client_fd);
    // log_request(*conn);
  } else if (bytes_received == -1) {
    DEBUG_LOG("Receive error on fd " << connection->client_fd);
    cleanup_connection(connection->client_fd);
    return;
  }
  DEBUG_LOG("Received " << bytes_received
                        << " bytes from fd: " << connection->client_fd);

  HTTPRequest &request = connection->m_Request;
  HTTPParser::parse(request, buffer, bytes_received);
  if (request.isComplete()) {
    connection->m_State = Connection::RESPONSE_PROCESSING;
    connection->init_response();
    this->handle_client_response(connection);
  }
}

void CGIHandler::cleanup_by_fd(int fd) {
  auto it = processes.find(fd);
  if (it != processes.end()) {
    cleanup(it->second, true);
  }
}
void WebServer::handle_client(Connection &conn) {

  if (events & EPOLL_ERRORS) {
    if (cgi.is_cgi_socket(fd)) {
      cgi.cleanup_by_fd(fd);
    } else {
      cleanup_connection(fd);
    }
    return;
  }
  // if (conn.events & EPOLL_ERRORS) {
  //   // if (cgi !)
  //   DEBUG_LOG("Connection closed by client (fd: "
  //             << fd << ")" << " -> EPOLLRDHUP || EPOLLHUP Happend!");
  //   cleanup_connection(conn.fd);
  //   return;
  // }

  Connection *conn = find_connection(fd);
  if (!conn)
    return; // handle error;

  if (conn->m_State == Connection::REQUEST_PARSING && (events & EPOLLIN)) {
    this->handle_client_request(conn);
  } else if (conn->m_State == Connection::RESPONSE_PROCESSING &&
             (events & EPOLLOUT)) {
    this->handle_client_response(fd, conn);
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

Connection *WebServer::find_connection(int fd) {
  for (std::list<Connection *>::iterator it = connections.begin();
       it != connections.end(); ++it) {
    if ((*it)->client_fd == fd)
      return *it;
  }
  return NULL;
}

void WebServer::cleanup_connection(int fd) {
  DEBUG_LOG("[Cleanup] Starting cleanup for fd: " << fd);

  for (std::list<Connection *>::iterator it = connections.begin();
       it != connections.end(); ++it) {
    if ((*it)->client_fd == fd) {
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
