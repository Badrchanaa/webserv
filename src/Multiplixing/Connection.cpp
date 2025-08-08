#include "Connection.hpp"

// Connection::Connection(CGIHandler &cgihandler, std::vector<ConfigServer> &servers, Config &conf, int f)
//       : m_State(REQUEST_PARSING), config(conf), Cgihandler(cgihandler), client_fd(f), m_Request(servers), hasEvent(false), cgiEvent(false) , socketEvent(false), events(false) {

//       std::cout << "hello test test" << std::endl;
//   }
// Connection::Connection(std::vector<ConfigServer> &servers, int f)
//       : cgi_Added(false) , client_Added(false), m_State(REQUEST_PARSING),
//       servers(servers), client_fd(f), m_Request(servers), hasEvent(false),
//       cgiEvent(false) , socketEvent(false), events(0),
//       last_activity(std::time(NULL)) {
// 
//   }
Connection::Connection(std::vector<ConfigServer> &servers, int f)
    : cgi_Added(false), client_Added(false), m_State(REQUEST_PARSING),
     servers(servers), cgi_stderr_Added(false), cgiStderrEvent(false),
     client_fd(f), m_Request(servers), hasEvent(false), cgiEvent(false),
     socketEvent(false), events(0), last_activity(std::time(NULL)) {
}
  // Connection(CGIHandler &cgihandler, Config &conf, int f,
  //            struct sockaddr_storage a)
  // : client_fd(f), state(REQUEST_PARSING), addr(a) {}


// void Connection::resetEvents()
// {
//   this->hasEvent = false;
//   this->socketEvent = false;
//   this->cgiEvent = false;
//   this->events = 0;
// }

void Connection::resetEvents() {
    this->hasEvent = false;
    this->socketEvent = false;
    this->cgiEvent = false;
    this->cgiStderrEvent = false;  // Reset stderr event
    this->events = 0;
}
void Connection::reset() {
    m_Response.reset();

    if (m_Response.getCgiFd() != -1) {
        // close(m_Response.getCgiFd());
        // close(m_Response.getCgiStderrFd());
        m_Response.cleanupCgi(false);
    }

    m_Request.reset();
    m_State = REQUEST_PARSING;
}


bool Connection::keepAlive() { return this->m_KeepAlive; }


  void Connection::init_response(EpollManager& epollManager, CGIHandler& cgiHandler) {
  this->m_State = RESPONSE_PROCESSING;
  (void) epollManager;
  const ConfigServer *configServer;
  configServer = m_Request.getServer(); 
  if (!configServer)
  {
    std::cout << "CANNOT FIND CONFIGSERVER" << std::endl;
    configServer = &(Config::getServerByName(servers, ""));
  }
  else
  {
    std::cout << "CONFIGSERVER FOUND" << std::endl;
  }
  
  // Initialize response and CGI
  this->m_Response.init(this->m_Request, cgiHandler, configServer, this->client_fd);

  this->m_KeepAlive = this->m_Response.isKeepAlive();
}
