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

bool WebServer::try_attach_to_existing_listener(const ConfigServer &new_server,
                                                int port) {
  typedef std::map<int, std::vector<ConfigServer> > ListenerMap;

  for (ListenerMap::iterator it = listener_map.begin();
       it != listener_map.end(); ++it) {
    // int s = 0;
    // std::cout << "MapSize : " << listener_map.size() <<  "[First] -> " <<
    // it->first  << std::endl;
    const int existing_fd = it->first;
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    if (getsockname(existing_fd, (struct sockaddr *)&addr, &addr_len) == -1) {
      DEBUG_LOG("getsockname failed: " << strerror(errno));
      continue;
    }
    const int flags = NI_NUMERICHOST | NI_NUMERICSERV;
    if (getnameinfo((struct sockaddr *)&addr, addr_len, hbuf, sizeof(hbuf),
                    sbuf, sizeof(sbuf), flags) != 0) {
      // DEBUG_LOG("getnameinfo failed: " << gai_strerror(s));
      DEBUG_LOG("getnameinfo failed: ");
      continue;
    }

    const int existing_port = atoi(sbuf);
    const std::string current_host =
        new_server.host.empty() ? "0.0.0.0" : new_server.host;

    // Compare normalized host:port
    if (existing_port == port &&
        (strcmp(hbuf, current_host.c_str()) == 0 ||
         (current_host == "0.0.0.0" && strcmp(hbuf, "::") == 0))) {

      std::set<std::string> existing_names;
      // std::cout << "---------------------------------------------------------
      // " << existing_port << std::endl;
      for (std::vector<ConfigServer>::const_iterator sit = it->second.begin();
           sit != it->second.end(); ++sit) {
        existing_names.insert(sit->server_names.begin(),
                              sit->server_names.end());
      }

      bool conflict = false;
      for (std::vector<std::string>::const_iterator nit =
               new_server.server_names.begin();
           nit != new_server.server_names.end(); ++nit) {
        if (existing_names.find(*nit) != existing_names.end()) {
          // std::cout << "nit : " << *nit << std::endl;
          conflict = true;
          break;
        }
      }
      // std::cout <<
      // "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ " <<
      // conflict << std::endl;

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
    ConfigServer &server = config.getServer(i);

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

      for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        FileDescriptor temp_fd(socket(
            p->ai_family, p->ai_socktype | SOCK_NONBLOCK, p->ai_protocol));
        if (temp_fd.fd == -1) {
          DEBUG_LOG("Socket creation failed: " << strerror(errno));
          continue;
        }

        int yes = 1;
        if (setsockopt(temp_fd.fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
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
          // std::cout << "Port -------> " << *port_it << " | fd -------> " <<
          // fd << std::endl;
          listener_map[fd].push_back(server);
          // epoll.add_fd(fd, EPOLL_READ | EPOLL_WRITE);
          epoll.add_fd(fd, EPOLL_READ);
          DEBUG_LOG("Successfully bound to " << server.host << ":" << port);
          break;
        } else {
          if (errno == EADDRINUSE) {
            if (try_attach_to_existing_listener(server, port)) {
              DEBUG_LOG("Attached virtual host to existing listener on port "
                        << port);
              break;
            } else {
              freeaddrinfo(res);
              std::ostringstream oss;
              oss << "Port " << port
                  << " already in use with conflicting server names";
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
  // for (std::list<Connection *>::iterator it = connections.begin();
  //      it != connections.end(); ++it) {
  //   delete *it;
  //   this->connections.erase(it);
  // }
}

WebServer::WebServer() : running(true) {
  config.ParseConfigFile(DEFAULT_PATH);
  DEBUG_LOG("Initializing web server...");
  create_listeners();
  this->cgi.epoll_fd = this->epoll.epfd;
  DEBUG_LOG("Server initialized. Entering main event loop.");
}


// In src/Multiplixing/WebServer.cpp
Connection &WebServer::getClientConnection(int fd, uint32_t events) {
  (void)events;
  for (std::list<Connection *>::iterator it = connections.begin();
       it != connections.end(); ++it) {
    Connection &conn = *(*it);

    // Check if the event is for client_fd or CGI fd
    if (conn.client_fd == fd) {
      conn.socketEvent = true;
      // return conn;
    }
    if (conn.m_Response.getCgiFd() == fd) { // Add this check
      conn.cgiEvent = true;
      // return conn;
    }
    if (conn.socketEvent || conn.cgiEvent)
      return conn;
  }
  throw std::runtime_error("Connection not found");
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
        // conn.hasEvent = false;
        Connection &conn =
            this->getClientConnection(events[i].data.fd, events[i].events);
        conn.hasEvent = true;
        conn.events |= events[i].events;
        DEBUG_LOG("events|= cgifd : " << conn.m_Response.getCgiFd() << " | client_fd : " << conn.client_fd << " | Events --->  " << events[i].data.fd << " with events: " << epoll.format_events(events[i].events));
        // this->handle_client(events[i].data.fd, events[i].events);
      }
    }

    for (std::list<Connection *>::iterator it = this->connections.begin();
         it != connections.end();) {
      std::cout << "iterator " << std::endl;
      if ((*it)->hasEvent) {
        bool connectionDeleted = this->handle_client(*(*it));
        DEBUG_LOG("[hashEvent] : " << (*it)->m_Response.getCgiFd() << " | client_fd : " << (*it)->client_fd << " | connectionDeleted " << connectionDeleted << " with events: " << epoll.format_events((*it)->events));
        if (!connectionDeleted) {
          std::cout << "i am in if of loop" << std::endl;
          (*it)->resetEvents();
          it++;
        } else
        {
          std::cout << "i am in else of loop" << std::endl;
      
          cleanup_connection(it);
        }
      } else
        it++;
    }
      std::cout << "HI ------------ " << std::endl;


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

  Connection *conn = new Connection(server_conf, new_fd);
  connections.push_back(conn);
  // epoll.add_fd(new_fd, EPOLL_READ);
  epoll.add_fd(new_fd, EPOLL_READ | EPOLL_WRITE);

  log_connection(client_addr);
}

bool WebServer::handle_client_response(Connection &conn) {
    HTTPResponse &response = conn.m_Response;
    bool shouldDelete = false;
    int cgi_fd = response.getCgiFd();

    // Process the response state machine
    response.resume(conn.cgiEvent, conn.socketEvent);

    // Handle completion
    if (response.isDone()) {
        // Clean up CGI resources first
        if (cgi_fd > 0) {
            try {
                epoll.remove_fd(cgi_fd);
                close(cgi_fd);
                response.cleanupCgi();
            } catch (...) {
                DEBUG_LOG("Error cleaning up CGI fd: " << cgi_fd);
            }
        }

        // Determine if we should close the connection
        if (!response.isKeepAlive()) {
            shouldDelete = true;
        } else {
            // Reset connection for keep-alive
            conn.reset();
            try {
                // epoll.mod_fd(conn.client_fd, EPOLLIN | EPOLLONESHOT);
                if (!conn.client_Added){
                  epoll.add_fd(conn.client_fd, EPOLLIN | EPOLLONESHOT);
                  conn.client_Added = 1;
                }
                else
                  epoll.mod_fd(conn.client_fd, EPOLLIN | EPOLLONESHOT);
            } catch (...) {
                DEBUG_LOG("Error resetting client fd: " << conn.client_fd);
                shouldDelete = true;
            }
        }
    }

    if (response.hasCgi() && cgi_fd > 0) {
        HTTPResponse::pollState currState = response.getPollState();
        
        try {
            switch (currState) {
                case HTTPResponse::CGI_READ:
                  if (conn.client_Added)
                  {
                    conn.client_Added = 0;
                    epoll.remove_fd(conn.client_fd);
                  }
                  if (!conn.cgi_Added){
                    conn.cgi_Added = 1;
                    epoll.add_fd(cgi_fd, EPOLLIN | EPOLLONESHOT);
                  }
                  else
                    epoll.mod_fd(cgi_fd, EPOLLIN | EPOLLONESHOT);
                  break;
                    
                case HTTPResponse::CGI_WRITE:
                  if (conn.client_Added)
                  {
                    conn.client_Added = 0;
                    epoll.remove_fd(conn.client_fd);
                  }
                  if (!conn.cgi_Added){
                    conn.cgi_Added = 1;
                    epoll.add_fd(cgi_fd, EPOLLOUT | EPOLLONESHOT);
                  }
                  else
                    epoll.mod_fd(cgi_fd, EPOLLOUT | EPOLLONESHOT);
                  break;
                    
                case HTTPResponse::SOCKET_WRITE:
                  if (conn.cgi_Added)
                    epoll.remove_fd(cgi_fd);
                  if (!conn.client_Added){
                    epoll.add_fd(conn.client_fd, EPOLLOUT | EPOLLONESHOT);
                    conn.client_Added = 1;
                  }
                  else
                    epoll.mod_fd(conn.client_fd, EPOLLOUT | EPOLLONESHOT);
                  break;
                    
                default:
                    break;
            }
        } catch (const std::exception& e) {
            DEBUG_LOG("Epoll error: " << e.what());
            close_fds(conn);
            shouldDelete = true;
        }
    }

    return shouldDelete;
}


void WebServer::close_fds(Connection &connection) {
    try {
        if (connection.client_fd > 0) {
            epoll.remove_fd(connection.client_fd);
            close(connection.client_fd);
            connection.client_fd = -1;  // Invalidate immediately
        }
        int cgi_fd = connection.m_Response.getCgiFd();
        if (cgi_fd > 0) {
            epoll.remove_fd(cgi_fd);
            close(cgi_fd);
            connection.m_Response.cleanupCgi();
        }
    } catch (...) {
        DEBUG_LOG("Error closing FDs");
    }
}

bool WebServer::handle_client_request(Connection &connection) {
  char buffer[BUFFER_SIZE];
  ssize_t bytes_received;
  bool isDeleted;

  isDeleted = false;
  bytes_received = recv(connection.client_fd, buffer, sizeof(buffer), 0);
  if (bytes_received == 0) {
    // Should send response ??
    connection.init_response(this->epoll, this->cgi);
    isDeleted = this->handle_client_response(connection);
    DEBUG_LOG("Connection closed by client (fd: " << connection.client_fd << ")");
    close_fds(connection);
    return true;
  } else if (bytes_received == -1) {
    DEBUG_LOG("Receive error on fd " << connection.client_fd);
    close_fds(connection);
    return true;
  }
  DEBUG_LOG("Received " << bytes_received
                        << " bytes from fd: " << connection.client_fd);

  HTTPRequest &request = connection.m_Request;
  HTTPParser::parse(request, buffer, bytes_received);
  if (request.isComplete()) {
    connection.m_State = Connection::RESPONSE_PROCESSING;
    connection.init_response(epoll, cgi);
    isDeleted = this->handle_client_response(connection);
  }
  return isDeleted;
}

bool WebServer::handle_client(Connection &conn) {

  bool isDeleted;
  int cgi_fd = -1;
  int client_fd = -1;
  isDeleted = false;
  if (conn.events & EPOLL_ERRORS) {
    // int fd = this->getCgiFdBasedOnClientFd(conn.client_fd);
    cgi_fd = conn.m_Response.getCgiFd();
    if (cgi_fd != -1 && conn.cgiEvent) {
      try{

      // this->epoll.remove_fd(cgi_fd);
      conn.m_Response.cleanupCgi();
      // close(cgi_fd);
      conn.cgiEvent = false; 
      }
      catch(...)
      {
        DEBUG_LOG("error in handle_client remove_fd\n");
      }
      // mybe should set 0 on cgi_sock of response object
    } 
    if (conn.socketEvent) {
      // client_fd = conn.client_fd;
      // epoll.remove_fd(client_fd);
      conn.socketEvent = false; 
      close_fds(conn);
      isDeleted = true;
    }
    DEBUG_LOG("Connection closed by client (fd: " << cgi_fd << " | " << client_fd << ")" << this->epoll.format_events(conn.events));
    return isDeleted; // hadle errors don't forget
  }

  if (conn.m_State == Connection::REQUEST_PARSING &&
      (conn.events & EPOLL_READ)) {
    isDeleted = this->handle_client_request(conn);
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


// void WebServer::cleanup_connection(std::list<Connection *>::iterator &it) {
//   Connection *connection = *it;
//
//
//   delete connection;
//   it = connections.erase(it);
// }

void WebServer::cleanup_connection(std::list<Connection *>::iterator &it) {
    Connection *conn = *it;
    if (conn->client_fd > 0) {
        epoll.remove_fd(conn->client_fd);
        close(conn->client_fd);
    }
    if (conn->m_Response.getCgiFd() > 0) {
        epoll.remove_fd(conn->m_Response.getCgiFd());
        close(conn->m_Response.getCgiFd());
    }
    delete conn;
    it = connections.erase(it);
}


int main() {
  try {
    WebServer server;
    server.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (const std::runtime_error &e) {
    std::cerr << "RUntime Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "SOMETHING WENT WRONG" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
