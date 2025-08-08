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

WebServer* WebServer::instance = NULL;


// Improve the existing cleanupListenerMap function
void cleanupListenerMap(std::map<int, std::vector<ConfigServer> >& listener_map) {
    for (std::map<int, std::vector<ConfigServer> >::iterator it = listener_map.begin();
         it != listener_map.end(); ++it) {
        
        std::vector<ConfigServer>& servers = it->second;
        
        for (std::vector<ConfigServer>::iterator vec_it = servers.begin();
             vec_it != servers.end(); ++vec_it) {
            
            // Clear all nested containers
            vec_it->ports.clear();
            vec_it->server_names.clear();
            vec_it->errors.clear();
            
            // Force deallocation using swap trick
            std::vector<int> empty_ports;
            vec_it->ports.swap(empty_ports);
            
            std::vector<std::string> empty_names;
            vec_it->server_names.swap(empty_names);
            
            std::map<int, std::string> empty_errors;
            vec_it->errors.swap(empty_errors);
            
            // Clear locations
            for (std::vector<Location>::iterator loc_it = vec_it->locations.begin();
                 loc_it != vec_it->locations.end(); ++loc_it) {
                loc_it->indexes.clear();
                loc_it->cgi.clear();
                
                // Force deallocation
                std::vector<std::string> empty_indexes;
                loc_it->indexes.swap(empty_indexes);
                
                std::map<std::string, std::string> empty_cgi;
                loc_it->cgi.swap(empty_cgi);
            }
            vec_it->locations.clear();
            
            // Force deallocation of locations vector
            std::vector<Location> empty_locations;
            vec_it->locations.swap(empty_locations);
        }
        servers.clear();
        
        // Force deallocation of servers vector
        std::vector<ConfigServer> empty_servers;
        servers.swap(empty_servers);
    }
    listener_map.clear();
}

// void cleanupListenerMap(std::map<int, std::vector<ConfigServer> > &listener_map) {
//     for (std::map<int, std::vector<ConfigServer> >::iterator it = listener_map.begin();
//          it != listener_map.end(); ++it) {
//         std::vector<ConfigServer> &servers = it->second;

//         for (std::vector<ConfigServer>::iterator vec_it = servers.begin();
//              vec_it != servers.end(); ++vec_it) {
//             vec_it->ports.clear();
//             vec_it->server_names.clear();
//             vec_it->errors.clear();

//             for (std::vector<Location>::iterator loc_it = vec_it->locations.begin();
//                  loc_it != vec_it->locations.end(); ++loc_it) {
//                 loc_it->indexes.clear();
//                 loc_it->cgi.clear();
//             }
//             vec_it->locations.clear();
//         }
//         servers.clear();
//     }
//     listener_map.clear();
// }


void WebServer::sig_interrupt_handler(int signum) {
    if (instance != NULL) {
        instance->handle_sigint(signum);
    }
    else
      std::cerr << "no instance found" << std::endl;
    delete instance;
    std::cerr <<"No Connection, Cleanup completed. Exiting..." << std::endl;
    exit(EXIT_FAILURE);
    // throw std::runtime_error("No Connection, Cleanup completed. Exiting...\n");
}



// void WebServer::handle_sigint(int signum) {
//     (void)signum;
//     std::cout << "Signal received, cleaning up..." << std::endl;
    
//     // Set running to false to exit main loop
//     running = false;
    
//     for (std::list<Connection *>::iterator it = connections.begin(); it != connections.end();) {
//         cleanup_connection(it);
//     }
//     connections.clear();

//     for (std::vector<FileDescriptor*>::iterator it = listener_descriptors.begin(); it != listener_descriptors.end(); ++it) {
//         if (*it) {
//             if ((*it)->fd > 0) {
//                 bool flag = true;
//                 epoll.remove_fd(flag, (*it)->fd);
//                 close((*it)->fd);
//             }
//             delete *it;
//         }
//     }
//     listener_descriptors.clear();
//     cleanupListenerMap(listener_map);


//     for (std::vector<ConfigServer>::iterator it = config.getServers().begin(); 
//         it != config.getServers().end(); ++it) 
//     {
//         ConfigServer server = *it;
//         server.ports.clear();
//         server.server_names.clear();
//         server.errors.clear();

//         for (std::vector<Location>::iterator loc_it = server.locations.begin(); 
//             loc_it != server.locations.end(); ++loc_it) 
//         {
//             loc_it->indexes.clear();
//             loc_it->cgi.clear();
//         }

//         server.locations.clear();
//     }
//     this->config.getServers().clear();
    
//     if (epoll.epfd > 0) {
//         close(epoll.epfd);
//         epoll.epfd = -1;
//     }
    
//     // exit(0);
//     // throw std::runtime_error("Cleanup completed. Exiting...\n");
// }

// In WebServer::handle_sigint()
void WebServer::handle_sigint(int signum) {
    (void)signum;
    std::cout << "Signal received, cleaning up..." << std::endl;
    
    running = false;
    
    // Force cleanup all connections
    std::list<Connection*>::iterator it = connections.begin();
    while (it != connections.end()) {
        Connection* conn = *it;
        
        // Close all file descriptors
        if (conn->client_fd > 0) {
            if (conn->client_Added) {
                bool flag = true;
                epoll.remove_fd(flag, conn->client_fd);
            }
            close(conn->client_fd);
        }
        
        if (conn->m_Response.hasCgi()) {
            conn->m_Response.cleanupCgi(true);
        }
        
        conn->m_Request.forceCleanup();
        conn->m_Response.forceCleanup();

        delete conn;
        it = connections.erase(it);
    }
    connections.clear();
    
    // Clean up other resources
    for (std::vector<FileDescriptor*>::iterator it = listener_descriptors.begin();
         it != listener_descriptors.end(); ++it) {
        if (*it) {
            if ((*it)->fd > 0) {
                bool flag = true;
                epoll.remove_fd(flag, (*it)->fd);
                close((*it)->fd);
            }
            delete *it;
        }
    }
    listener_descriptors.clear();
    
    cleanupListenerMap(listener_map);
    
    // Force cleanup of config servers vector
    std::vector<ConfigServer>& servers = config.getServers();
    for (std::vector<ConfigServer>::iterator it = servers.begin(); 
         it != servers.end(); ++it) {
        
        it->ports.clear();
        it->server_names.clear();
        it->errors.clear();
        it->locations.clear();
        
        // Force deallocation using swap trick
        std::vector<int> empty_ports;
        it->ports.swap(empty_ports);
        
        std::vector<std::string> empty_names;
        it->server_names.swap(empty_names);
        
        std::map<int, std::string> empty_errors;
        it->errors.swap(empty_errors);
        
        std::vector<Location> empty_locations;
        it->locations.swap(empty_locations);
    }
    servers.clear();
    
    // Force deallocation of main servers vector
    std::vector<ConfigServer> empty_servers;
    servers.swap(empty_servers);
    
    if (epoll.epfd > 0) {
        close(epoll.epfd);
        epoll.epfd = -1;
    }
}

void WebServer::setupSignalHandler() {
    signal(SIGINT, WebServer::sig_interrupt_handler);
    signal(SIGTERM, WebServer::sig_interrupt_handler);
    signal(SIGPIPE, SIG_IGN); // Ignore broken pipe signals
}



bool WebServer::try_attach_to_existing_listener(const ConfigServer &new_server,
                                                int port) {
  typedef std::map<int, std::vector<ConfigServer> > ListenerMap;

  for (ListenerMap::iterator it = listener_map.begin();
       it != listener_map.end(); ++it) {
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
          bool test = false;
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
            std::cout << "hello 1 : " << fd  << std::endl;
          epoll.add_fd(test, fd, EPOLL_READ);
          
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

// In WebServer.cpp - Improved destructor
WebServer::~WebServer() {
    // Clean up connections first with explicit iteration
    std::list<Connection*>::iterator it = connections.begin();
    while (it != connections.end()) {
      Connection* conn = *it;
      
      // Close client socket
      if (conn->client_fd > 0) {
          if (conn->client_Added) {
              bool flag = true;
              epoll.remove_fd(flag, conn->client_fd);
          }
          close(conn->client_fd);
      }
      
      // Close CGI sockets
      if (conn->m_Response.hasCgi()) {
          int cgi_fd = conn->m_Response.getCgiFd();
          int cgi_stderr_fd = conn->m_Response.getCgiStderrFd();
          
          if (cgi_fd > 0 && conn->cgi_Added) {
              bool flag = true;
              epoll.remove_fd(flag, cgi_fd);
          }
          if (cgi_stderr_fd > 0 && conn->cgi_stderr_Added) {
              bool flag = true;
              epoll.remove_fd(flag, cgi_stderr_fd);
          }
          conn->m_Response.cleanupCgi(true);
      }
      conn->m_Request.forceCleanup();
      conn->m_Response.forceCleanup();

      delete conn;
        it = connections.erase(it);
    }
    connections.clear();
    
    // Clean up listeners
    for (std::vector<FileDescriptor*>::iterator it = listener_descriptors.begin();
         it != listener_descriptors.end(); ++it) {
        if (*it && (*it)->fd > 0) {
            bool flag = true;
            epoll.remove_fd(flag, (*it)->fd);
            close((*it)->fd);
            (*it)->fd = -1;
        }
        delete *it;
    }
    listener_descriptors.clear();
    
    // Force vector deallocation using swap trick (C++98 compatible)
    std::vector<FileDescriptor*> empty_listeners;
    listener_descriptors.swap(empty_listeners);
    
    // Clean up listener map
    cleanupListenerMap(listener_map);
    
    // Close epoll
    if (epoll.epfd > 0) {
        close(epoll.epfd);
        epoll.epfd = -1;
    }
}

// WebServer::~WebServer() {
//   // for (std::vector<FileDescriptor *>::iterator it =
//   //          listener_descriptors.begin();
//   //      it != listener_descriptors.end(); ++it) {
//   //   delete *it;
//   // }
//   for (std::vector<FileDescriptor *>::iterator it = listener_descriptors.begin();
//         it != listener_descriptors.end(); ++it) {
//       if ((*it)->fd > 0) {
//           bool flag = true;
//           epoll.remove_fd(flag, (*it)->fd); // Remove from epoll first
//           close((*it)->fd);
//           (*it)->fd = -1;
//       }
//     delete *it;
//   }
//   for (std::list<Connection *>::iterator it = connections.begin();
//        it != connections.end();) {
//     this->connections.erase(it);
//   }
//   if (epoll.epfd > 0) {
//       close(epoll.epfd);
//       epoll.epfd = -1;
//   }
// }

// WebServer::WebServer(const char *FileCofig) : running(true) {
//   config.creatDefaultServer();
//   config.ParseConfigFile(FileCofig);
//   std::cout << "===========================================================" << std::endl;
//   this->config.printServersWithLocations();
//   std::cout << "===========================================================" << std::endl;
//   std::cout << "ServersNumber : " << config.ServersNumber() << std::endl;
//   DEBUG_LOG("Initializing web server...");
//   create_listeners();
//   this->cgi.epoll_fd = this->epoll.epfd;
//   DEBUG_LOG("Server initialized. Entering main event loop.");
// }
WebServer::WebServer(const char *FileCofig) : running(true) {
    if (instance)
        throw std::runtime_error("Instance Already Exists !");
    instance = this;
    config.creatDefaultServer();
    config.ParseConfigFile(FileCofig);
    std::cout << "===========================================================" << std::endl;
    this->config.printServersWithLocations();
    std::cout << "===========================================================" << std::endl;
    std::cout << "ServersNumber : " << config.ServersNumber() << std::endl;
    DEBUG_LOG("Initializing web server...");
    create_listeners();
    this->cgi.epoll_fd = this->epoll.epfd;
    DEBUG_LOG("Server initialized. Entering main event loop.");
}


Connection &WebServer::getClientConnection(int fd, uint32_t events) {
  (void)events;
  for (std::list<Connection *>::iterator it = connections.begin();
       it != connections.end(); ++it) {
    Connection &conn = *(*it);
    // conn.resetEvents();
    if (conn.client_fd == fd)
    {
      conn.socketEvent = true;
      // std::cout << "[Connection: " << conn.client_fd << "] EVENT ON CLIENT FD: " << epoll.format_events(events) << std::endl;
    }
    // else
    //   conn.socketEvent = false;
    if (conn.m_Response.getCgiFd() == fd) {
      conn.cgiEvent = true;
      // std::cout << "[Connection: " << conn.client_fd << "] EVENT ON CGI STDOUT FD: " << epoll.format_events(events) << std::endl;
    }
    if (conn.m_Response.getCgiStderrFd() == fd) {
      conn.cgiStderrEvent = true;  // Add this flag to Connection class
      std::cout << "[Connection: " << conn.client_fd << "] EVENT ON CGI STDERR FD: " << epoll.format_events(events) << std::endl;
    }
    
    if ((conn.client_fd == fd && conn.socketEvent) || 
        (conn.m_Response.getCgiFd() == fd && conn.cgiEvent) ||
        (conn.m_Response.getCgiStderrFd() == fd && conn.cgiStderrEvent)) {
        return conn;
    }
  }
  throw std::runtime_error("Connection not found");
}

void WebServer::run() {
  struct epoll_event events[MAX_EVENTS];
  DEBUG_LOG("Starting main event loop");
  // signal(SIGINT, WebServer::sig_interrupt_handler);
  while (running) {
    // std::cout << "WAITING FOR EVENTS" << std::endl;
    // int n = epoll_wait(epoll.epfd, events, MAX_EVENTS, -1);
    int n = epoll_wait(epoll.epfd, events, MAX_EVENTS, TIMEOUT_SEC * 1000);
    DEBUG_LOG("Epoll_wait returned " << n << " events");

    if (n == -1 && errno != EINTR)
      throw std::runtime_error("epoll_wait failed");

    std::time_t current_time = std::time(NULL); // Capture current time once per loop iteration
    // std::cout << "current time : " << current_time << std::endl;
    // sleep(10);
    // std::cout << "After sleep : " << std::time(NULL) << std::endl;

    for (int i = 0; i < n; ++i) {
      if (listener_map.find(events[i].data.fd) != listener_map.end()) {
        DEBUG_LOG("New connection on listener fd: " << events[i].data.fd);
        this->accept_connections(events[i].data.fd);
      } else {
        Connection &conn = this->getClientConnection(events[i].data.fd, events[i].events);
        conn.hasEvent = true;
        conn.events |= events[i].events;

      }
    }


    for (std::list<Connection *>::iterator it = this->connections.begin(); it != connections.end();)
    {
      Connection *conn = *it;
      if ((*it)->hasEvent) {
        DEBUG_LOG("[cgiFd] : " << conn->m_Response.getCgiFd() << " | client_fd : "
          << conn->client_fd  << " with events: " << epoll.format_events(conn->events) << std::endl);
        bool connectionDeleted = this->handle_client(*conn);
        // std::cout << "connectionDeleted " << connectionDeleted  << std::endl;
        if (!connectionDeleted) { // it++
          // std::cout << "FIRST TIMEOUT CASE" << std::endl;
          if (current_time - conn->last_activity >= TIMEOUT_SEC) 
            handle_connection_timeout(it);
          else {
            conn->resetEvents();
            it++;
          }
        } else {
          // 
          cleanup_connection(it);
        }
      }
      else
      {
          // exit(1);
          // conn->m_Response.setError(HTTPResponse::SERVER_ERROR);
          // close_fds(*conn);
          // cleanup_connection(it);
        // std::cout << "SECOND TIMEOUT CASE" << std::endl;
        if (current_time - conn->last_activity >= TIMEOUT_SEC) 
          handle_connection_timeout(it);
        else
          it++;

      }
    }
    // std::cout << "RUN END ------------ " << std::endl;
  }
}

void WebServer::handle_connection_timeout(std::list<Connection *>::iterator &it)
{
  std::cout << "CONNECTION TIMEOUT" << std::endl;
    Connection *conn = *it;
    if (conn->cgi_Added) // Timeout in CGI
    {
      epoll.remove_fd(conn->cgi_Added, conn->m_Response.getCgiFd());
      if (conn->cgi_stderr_Added)
        epoll.remove_fd(conn->cgi_stderr_Added, conn->m_Response.getCgiStderrFd());
      epoll.add_fd(conn->client_Added, conn->client_fd, EPOLL_READ | EPOLL_WRITE);
      conn->m_Response.cleanupCgi(true);
      conn->m_Response.setError(HTTPResponse::GATEWAY_TIMEOUT);
      std::cout << "AFTER SET ERROR" << std::endl;
    }
    else if (conn->client_Added){
        close_fds(*conn);
        return cleanup_connection(it);
    }
    std::cout << "TIMEOUT FUNCTION END" << std::endl;
    conn->last_activity = std::time(NULL);
    it++;
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
  conn->last_activity = std::time(NULL);

  connections.push_back(conn);
  // epoll.add_fd(new_fd, EPOLL_READ);
  std::cout << "hello 0" << std::endl;
  std::cout << "4444444444444444444444444444444444" << std::endl;
  epoll.add_fd(conn->client_Added , new_fd, EPOLL_READ | EPOLL_WRITE );
  // epoll.add_fd(new_fd, EPOLL_READ );
  std::cout << "4444444444444444444444444444444444" << std::endl;

  log_connection(client_addr);
}

bool WebServer::handle_client_response(Connection &conn) {
    HTTPResponse &response = conn.m_Response;
    bool shouldDelete = false;
    int cgi_fd = response.getCgiFd();
    int cgi_stderr_fd = response.getCgiStderrFd(); // this
    HTTPResponse::pollState state = response.getPollState();
    
    // handle stderr events
    if (conn.cgiStderrEvent && cgi_stderr_fd > 0) {
        response.processStderr();
    }
    
    if (conn.cgiEvent && conn.events & EPOLLHUP)
        response.setCgiDone();
    else if (conn.cgiEvent) {// if cgiEvent and Response in READ or WRITE
        if ((state == HTTPResponse::CGI_READ && !(conn.events & EPOLLIN)) ||
            (state == HTTPResponse::CGI_WRITE && !(conn.events & EPOLLOUT))) {
            return shouldDelete;
        }
    }
    
    if (state == !HTTPResponse::SOCKET_WRITE && conn.socketEvent)
        return shouldDelete;

    conn.last_activity = std::time(NULL);
    response.resume(conn.cgiEvent, conn.socketEvent);

    if (response.isDone()) {
        if (response.hasCgi()) {
            try {
                if (conn.cgi_Added) {
                    epoll.remove_fd(conn.cgi_Added, cgi_fd);
                }
                if (conn.cgi_stderr_Added) { 
                    epoll.remove_fd(conn.cgi_stderr_Added, cgi_stderr_fd);
                }
                // close(cgi_fd); 
                // close(cgi_stderr_fd);
                // kan klosihom f cleanupCgi
                response.cleanupCgi(false);
            } catch (...) {
                DEBUG_LOG("Error cleaning up CGI fds");
            }
        }

        if (!response.isKeepAlive()) {
            shouldDelete = true;
            conn.m_Request.forceCleanup();
            conn.m_Response.forceCleanup();

        } else {
            std::cout << "NOT DONE" << std::endl;
            conn.reset();
            try {
                // epoll.mod_fd(conn.client_fd, EPOLLIN | EPOLLONESHOT);
                if (!conn.client_Added) {
                    epoll.add_fd(conn.client_Added, conn.client_fd, EPOLLOUT | EPOLLIN);
                } else {
                    epoll.mod_fd(conn.client_fd, EPOLLOUT | EPOLLIN);
                }
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
                    std::cout << "CONN in CGI_READ" << std::endl;
                    if (conn.client_Added) {
                        epoll.remove_fd(conn.client_Added, conn.client_fd);
                    }
                    if (!conn.cgi_Added) {
                        epoll.add_fd(conn.cgi_Added, cgi_fd, EPOLLOUT | EPOLLIN);
                    } else {
                        epoll.mod_fd(cgi_fd, EPOLLOUT | EPOLLIN);
                    }
                    
                    if (!conn.cgi_stderr_Added && cgi_stderr_fd > 0) {
                        epoll.add_fd(conn.cgi_stderr_Added, cgi_stderr_fd, EPOLLIN);
                    }
                    break;
                    
                case HTTPResponse::CGI_WRITE:
                    if (conn.client_Added) {
                        epoll.remove_fd(conn.client_Added, conn.client_fd);
                    }
                    if (!conn.cgi_Added) {
                        epoll.add_fd(conn.cgi_Added, cgi_fd, EPOLLIN | EPOLLOUT);
                    } else {
                        epoll.mod_fd(cgi_fd, EPOLLIN | EPOLLOUT);
                    }
                    
                    if (!conn.cgi_stderr_Added && cgi_stderr_fd > 0) {
                        epoll.add_fd(conn.cgi_stderr_Added, cgi_stderr_fd, EPOLLIN);
                    }
                    break;
                    
                case HTTPResponse::SOCKET_WRITE:
                    if (conn.cgi_Added) {
                        epoll.remove_fd(conn.cgi_Added, cgi_fd);
                    }
                    if (conn.cgi_stderr_Added) {
                        epoll.remove_fd(conn.cgi_stderr_Added, cgi_stderr_fd);
                    }
                    if (!conn.client_Added) {
                        epoll.add_fd(conn.client_Added, conn.client_fd, EPOLLIN | EPOLLOUT);
                    } else {
                        epoll.mod_fd(conn.client_fd, EPOLLIN | EPOLLOUT);
                    }
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
          if (connection.client_Added)
          {
            epoll.remove_fd(connection.client_Added, connection.client_fd);
          }
          close(connection.client_fd);
          connection.client_fd = -1;  // Invalidate immediately
        }
        int cgi_fd = connection.m_Response.getCgiFd();
        if (cgi_fd > 0) {
          if (connection.cgi_Added)
            epoll.remove_fd(connection.cgi_Added, cgi_fd);
          if (connection.cgi_stderr_Added)
              epoll.remove_fd(connection.cgi_stderr_Added, connection.m_Response.getCgiStderrFd());
          // close(cgi_fd);
          connection.m_Response.cleanupCgi(false);
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
  connection.last_activity = std::time(NULL);
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
  HTTPParser::parseRequest(request, buffer, bytes_received);
  if (request.isParseComplete()) {
    connection.init_response(epoll, cgi);
    isDeleted = this->handle_client_response(connection);
  }
  return isDeleted;
}

bool WebServer::handle_client(Connection &conn) {

  bool isDeleted;
  int cgi_fd = -1;
  // int client_fd = -1;
  isDeleted = false;

  // std::cout << "ENTER HANDLE CLIENT fd: " << conn.client_fd << std::endl;
  if (conn.events & EPOLL_ERRORS)
  {
    cgi_fd = conn.m_Response.getCgiFd();
    if (conn.socketEvent)
    {
      conn.socketEvent = false; 
      close_fds(conn);
      isDeleted = true; // hadle errors don't forget
    return isDeleted; // hadle errors don't forget
    }
    // DEBUG_LOG("Connection closed by client (fd: " << cgi_fd << " | " << client_fd << ")" << this->epoll.format_events(conn.events));
  }

  if (conn.m_State == Connection::REQUEST_PARSING &&
      (conn.events & EPOLL_READ)) {
        std::cout << "CONNECTION IN REQUEST PARSING" << std::endl;
        isDeleted = this->handle_client_request(conn);
  } else if (conn.m_State == Connection::RESPONSE_PROCESSING) {
        // std::cout << "CONNECTION IN RESPONSE PROCESSING" << std::endl;
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


void WebServer::cleanup_connection(std::list<Connection*>::iterator& it) {
    Connection* conn = *it;
    
    if (conn->client_fd > 0) {
        if (conn->client_Added) {
            epoll.remove_fd(conn->client_Added, conn->client_fd);
        }
        close(conn->client_fd);
        conn->client_fd = -1;
    }
    
    if (conn->m_Response.hasCgi()) {
        int cgi_fd = conn->m_Response.getCgiFd();
        int cgi_stderr_fd = conn->m_Response.getCgiStderrFd();
        
        if (cgi_fd > 0 && conn->cgi_Added) {
            try {
                epoll.remove_fd(conn->cgi_Added, cgi_fd);
            } catch (...) {
            }
        }
        if (cgi_stderr_fd > 0 && conn->cgi_stderr_Added) {
            try {
                epoll.remove_fd(conn->cgi_stderr_Added, cgi_stderr_fd);
            } catch (...) {
            }
        }
        
        conn->m_Response.cleanupCgi(true);
    }
    
    // conn->m_Request.reset();
    // conn->m_Response.reset();
    
    delete conn;
    it = connections.erase(it);
}


int main(int argc, char **argv) {
    WebServer *server = NULL;
    try {
        if (argc != 2) {
            server = new WebServer(DEFAULT_PATH);
        } else {
            server = new WebServer(argv[1]);
        }
        
        server->setupSignalHandler();
        server->run();

        delete server;
        
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        delete server;
        return EXIT_FAILURE;
    } catch (const std::runtime_error &e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        delete server;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "SOMETHING WENT WRONG" << std::endl;
        delete server;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
