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

bool WebServer::try_attach_to_existing_listener(const ConfigServer& new_server, int port) {
    typedef std::map<int, std::vector<ConfigServer> > ListenerMap;
    
    for (ListenerMap::iterator it = listener_map.begin(); it != listener_map.end(); ++it) {
        // int s = 0;
        // std::cout << "MapSize : " << listener_map.size() <<  "[First] -> " << it->first  << std::endl;
        const int existing_fd = it->first;
        struct sockaddr_storage addr;
        socklen_t addr_len = sizeof(addr);
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

        if (getsockname(existing_fd, (struct sockaddr*)&addr, &addr_len) == -1) {
            DEBUG_LOG("getsockname failed: " << strerror(errno));
            continue;
        }
        const int flags = NI_NUMERICHOST | NI_NUMERICSERV;
        if (getnameinfo((struct sockaddr*)&addr, addr_len, 
                       hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), flags) != 0) {
            // DEBUG_LOG("getnameinfo failed: " << gai_strerror(s));
            DEBUG_LOG("getnameinfo failed: ");
            continue;
        }

        const int existing_port = atoi(sbuf);
        const std::string current_host = new_server.host.empty() ? "0.0.0.0" : new_server.host;

        // Compare normalized host:port
        if (existing_port == port && 
            (strcmp(hbuf, current_host.c_str()) == 0 || 
            (current_host == "0.0.0.0" && strcmp(hbuf, "::") == 0))) {
            
            std::set<std::string> existing_names;
            // std::cout << "--------------------------------------------------------- " << existing_port << std::endl;
            for (std::vector<ConfigServer>::const_iterator sit = it->second.begin();
                 sit != it->second.end(); ++sit) {
                existing_names.insert(sit->server_names.begin(), sit->server_names.end());
            }

            bool conflict = false;
            for (std::vector<std::string>::const_iterator nit = new_server.server_names.begin();
                 nit != new_server.server_names.end(); ++nit) {
                if (existing_names.find(*nit) != existing_names.end()) {
                  // std::cout << "nit : " << *nit << std::endl;
                    conflict = true;
                    break;
                }
            }
            // std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ " << conflict << std::endl;

            if (!conflict) {
                it->second.push_back(new_server);
                return true;
            }
        }
    }
    return false;
}

void WebServer::create_listeners() {
    DEBUG_LOG("Initializing network stack for all servers...");

    const int server_count = config.ServersNumber();
    // std::cout << "cout -------------------> " << server_count << std::endl;
    if (server_count == 0) {
        throw std::runtime_error("No server configurations found");
    }

    for (int i = 0; i < server_count; ++i) {
        ConfigServer& server = config.getServer(i);

        for (std::vector<int>::const_iterator port_it = server.ports.begin();
             port_it != server.ports.end(); ++port_it) {
            const int port = *port_it;
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

            DEBUG_LOG("Resolving addresses for " << server.host << ":" << port);
            std::ostringstream port_stream;
            port_stream << port;
            int status = getaddrinfo(server.host.empty() ? NULL : server.host.c_str(),
                                   port_stream.str().c_str(), &hints, &res);
            if (status != 0) {
                DEBUG_LOG("Address resolution failed: " << gai_strerror(status));
                continue;
            }

            for (struct addrinfo* p = res; p != NULL; p = p->ai_next) {
                FileDescriptor temp_fd(socket(p->ai_family, 
                                          p->ai_socktype | SOCK_NONBLOCK, 
                                          p->ai_protocol));
                if (temp_fd.fd == -1) {
                    DEBUG_LOG("Socket creation failed: " << strerror(errno));
                    continue;
                }

                int yes = 1;
                if (setsockopt(temp_fd.fd, SOL_SOCKET, SO_REUSEADDR, 
                              &yes, sizeof(int)) == -1) {
                    DEBUG_LOG("setsockopt failed: " << strerror(errno));
                    continue;
                }

                if (bind(temp_fd.fd, p->ai_addr, p->ai_addrlen) == 0) {
                    if (listen(temp_fd.fd, BACKLOG) == -1) {
                        DEBUG_LOG("listen failed: " << strerror(errno));
                        continue;
                    }

                    const int fd = temp_fd.release();
                    listener_descriptors.push_back(new FileDescriptor(fd));
                    // std::cout << "Port -------> " << *port_it << " | fd -------> " << fd << std::endl;
                    listener_map[fd].push_back(server);
                    epoll.add_fd(fd, EPOLL_READ | EPOLL_WRITE);
                    DEBUG_LOG("Successfully bound to " << server.host << ":" << port);
                    break;
                } else {
                    if (errno == EADDRINUSE) {
                        if (try_attach_to_existing_listener(server, port)) {
                            DEBUG_LOG("Attached virtual host to existing listener on port " << port);
                            break;
                        }
                        else {
                          freeaddrinfo(res);
                          std::ostringstream oss; oss << "Port " << port << " already in use with conflicting server names";
                          throw std::runtime_error(oss.str());
                        }
                    } else {
                        DEBUG_LOG("bind failed: " << strerror(errno));
                    }
                }
            }
            freeaddrinfo(res);
        }
    }

    if (listener_map.empty()) {
        throw std::runtime_error("All bind attempts failed");
    }
}
WebServer::~WebServer() {
  for (std::vector<FileDescriptor *>::iterator it =
           listener_descriptors.begin();
       it != listener_descriptors.end(); ++it) {
    delete *it;
  }
  for (std::list<Connection *>::iterator it = connections.begin();
       it != connections.end(); ++it) {
    delete *it;
    this->connections.erase(it);
  }
}

WebServer::WebServer() : running(true) {
  config.ParseConfigFile(DEFAULT_PATH);
  DEBUG_LOG("Initializing web server...");
  create_listeners();
  this->cgi.epoll_fd = this->epoll.epfd;
  DEBUG_LOG("Server initialized. Entering main event loop.");
}


Connection &WebServer::getClientConnection(int fd) {
  for (std::list<Connection *>::iterator it = connections.begin();
       it != connections.end(); ++it) {
    if ((*it)->client_fd == fd)
      (*it)->socketEvent = true;
    if ((*it)->m_Response.getCgiFd() == fd)
      (*it)->cgiEvent = true;
    if ((*it)->socketEvent || (*it)->cgiEvent)
      return *(*it);
  }
  throw std::runtime_error("Clinet Connection not found OR Cgi Connection not found");
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
      if (listener_map.find(events[i].data.fd) != listener_map.end()) {
        DEBUG_LOG("New connection on listener fd: " << events[i].data.fd);
        this->accept_connections(events[i].data.fd);
      } else {
        DEBUG_LOG("Processing event on fd: " << events[i].data.fd);
        Connection &conn = this->getClientConnection(events[i].data.fd);
        conn.hasEvent = true;
        conn.events |= events[i].events;
        // this->handle_client(events[i].data.fd, events[i].events);
      }
    }

    for (std::list<Connection *>::iterator it = this->connections.begin();
         it != connections.end();) {
      if ((*it)->hasEvent) {
        bool connectionDeleted = this->handle_client(*(*it));
        if (!connectionDeleted) {
          (*it)->resetEvents();
          it++;
        } else
          cleanup_connection(it);
      } else
        it++;
    }
  }
}

void WebServer::accept_connections(int listen_fd) {
  struct sockaddr_storage client_addr;
  socklen_t addr_size = sizeof(client_addr);
  std::vector<ConfigServer> &server_conf = this->listener_map[listen_fd];

  // map<fd, vector<server>>

  int new_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_size);
  if (new_fd == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return;
    throw std::runtime_error("accept failed");
  }

  Connection *conn = new Connection(this->cgi, server_conf, new_fd);
  connections.push_back(conn);
  epoll.add_fd(new_fd, EPOLL_READ);
  // epoll.add_fd(new_fd, EPOLL_READ | EPOLL_WRITE);

  log_connection(client_addr);
}

bool WebServer::handle_client_response(Connection &conn) {
  HTTPResponse &response = conn.m_Response;
  bool shouldDelete = false;

  response.resume(conn.cgiEvent, conn.socketEvent);
  if (response.isDone()) {
    if (!response.isKeepAlive()) {
      // cleanup_connection(conn.client_fd);
      shouldDelete = true;
    } else {
      conn.m_State = Connection::REQUEST_PARSING;
      /// nots this///
      epoll.mod_fd(conn.client_fd, EPOLL_READ | EPOLL_WRITE);
      conn.reset();
    }
  } else {
    HTTPResponse::pollState state = response.getPollState();
    // int cgi_sock = conn.Cgihandler.getCgiSocket(conn.client_fd);
    int cgi_sock = conn.m_Response.getCgiFd();

    if (state == HTTPResponse::CGI_WRITE) {
      // Switch monitoring to CGI socket for writing
      epoll.remove_fd(conn.client_fd);
      epoll.add_fd(cgi_sock, EPOLL_WRITE | EPOLL_READ);
      DEBUG_LOG("Switched to monitoring CGI socket (write)");
      
    } else if (state == HTTPResponse::CGI_READ) {
      // Switch monitoring to CGI socket for reading
      epoll.remove_fd(conn.client_fd);
      epoll.add_fd(cgi_sock, EPOLL_WRITE | EPOLL_READ);
      DEBUG_LOG("Switched to monitoring CGI socket (read)");
    } else {
      // Switch back to client socket for writing
      if (cgi_sock != -1) {
        epoll.remove_fd(cgi_sock);
        // response.clearCgiSocket();
      }
      epoll.mod_fd(conn.client_fd, EPOLL_READ | EPOLL_WRITE);
    }
  }
  return shouldDelete;
}

bool WebServer::handle_client_request(Connection &connection) {
  char buffer[BUFFER_SIZE];
  ssize_t bytes_received;
  bool isDeleted;

  isDeleted = false;
  bytes_received = recv(connection.client_fd, buffer, sizeof(buffer), 0);
  if (bytes_received == 0) {
    // Should send response ??
    connection.init_response();
    handle_client_response(connection);
    DEBUG_LOG("Connection closed by client (fd: " << connection.client_fd
                                                  << ")");
    // cleanup_connection(connection.client_fd);
    return true;
    // log_request(*conn);
  } else if (bytes_received == -1) {
    DEBUG_LOG("Receive error on fd " << connection.client_fd);
    // cleanup_connection(connection.client_fd);
    return true;
  }
  DEBUG_LOG("Received " << bytes_received
                        << " bytes from fd: " << connection.client_fd);

  HTTPRequest &request = connection.m_Request;
  HTTPParser::parse(request, buffer, bytes_received);
  if (request.isComplete()) {
    connection.m_State = Connection::RESPONSE_PROCESSING;
    connection.init_response();
    isDeleted = this->handle_client_response(connection);
  }
  return isDeleted;
}

bool WebServer::handle_client(Connection &conn) {

  bool isDeleted;
  int fd  = false;
  isDeleted = false;
  if (conn.events & EPOLL_ERRORS) {
    // int fd = this->getCgiFdBasedOnClientFd(conn.client_fd);
    fd = conn.m_Response.getCgiFd();
    if (conn.cgiEvent && fd) {
      cgi.cleanup_by_fd(fd);
    } else if (conn.socketEvent) {
      // cleanup_connection(conn.client_fd);
      fd = conn.client_fd;
      isDeleted = true;
    }
    DEBUG_LOG("Connection closed by client (fd: "
              << fd << ")" << this->epoll.format_events(conn.events));
    return isDeleted; // hadle errors don't forget
  }

  if (conn.m_State == Connection::REQUEST_PARSING &&
      (conn.events & EPOLL_READ)) {
    this->handle_client_request(conn);
  } else if (conn.m_State == Connection::RESPONSE_PROCESSING &&
             (conn.events & EPOLL_WRITE)) {
    isDeleted = this->handle_client_response(conn);
  }

  return isDeleted;
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

void WebServer::cleanup_connection(std::list<Connection *>::iterator &it) {
  DEBUG_LOG("[Cleanup] Starting cleanup for ");

  Connection *connection = *it;
  // DEBUG_LOG("[Cleanup] Removing fd " << fd << " from epoll");
  try {
    epoll.remove_fd(connection->client_fd); // Remove from epoll FIRST
  } catch (const std::exception &e) {
    DEBUG_LOG("[Cleanup] Error removing fd " << connection->client_fd << ": "
                                             << e.what());
  }

  delete connection; // Connection

  it = connections.erase(it);
  std::cout << "NUMBER OF CONNECTIONS: " << connections.size() << std::endl;

  return;
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
