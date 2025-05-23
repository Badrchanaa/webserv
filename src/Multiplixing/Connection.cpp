#include "Connection.hpp"

// Connection::Connection(CGIHandler &cgihandler, std::vector<ConfigServer> &servers, Config &conf, int f)
//       : m_State(REQUEST_PARSING), config(conf), Cgihandler(cgihandler), client_fd(f), m_Request(servers), hasEvent(false), cgiEvent(false) , socketEvent(false), events(false) {

//       std::cout << "hello test test" << std::endl;
//   }
Connection::Connection(std::vector<ConfigServer> &servers, int f)
      : cgi_Added(false) , client_Added(false), m_State(REQUEST_PARSING), client_fd(f), m_Request(servers), hasEvent(false), cgiEvent(false) , socketEvent(false), events(0), cgi_last_activity(false), client_last_activity(false) {

  }
  // Connection(CGIHandler &cgihandler, Config &conf, int f,
  //            struct sockaddr_storage a)
  // : client_fd(f), state(REQUEST_PARSING), addr(a) {}


void Connection::resetEvents()
{
  this->hasEvent = false;
  this->socketEvent = false;
  this->cgiEvent = false;
  this->events = 0;
}

void Connection::reset() {
    m_Response.reset();

    if (m_Response.getCgiFd() != -1) {
        close(m_Response.getCgiFd());
        m_Response.cleanupCgi();
    }

    m_Request.reset();
    m_State = REQUEST_PARSING;
}


bool Connection::keepAlive() { return this->m_KeepAlive; }


  void Connection::init_response(EpollManager& epollManager, CGIHandler& cgiHandler) {
  this->m_State = RESPONSE_PROCESSING;
  (void) epollManager;
  // Initialize response and CGI
  this->m_Response.init(this->m_Request, cgiHandler, this->m_Request.getServer(), this->client_fd);

  // Store CGI FD in response
  // if (m_Response.hasCgi()) {
  //   int cgi_fd = m_Response.getCgiFd();
  //   if (cgi_fd != -1) {
  //     // epollManager.remove_fd(client_fd);
  //     // epollManager.add_fd(cgi_fd, EPOLLIN | EPOLLOUT | EPOLLET);
  //     // this->epoll.remove_fd(this->client_fd);
  //     epollManager.mod_fd(client_fd, 0);
  //     epollManager.add_fd(cgi_fd, EPOLL_WRITE | EPOLL_READ);
  //     DEBUG_LOG("Monitoring CGI fd: " << cgi_fd);
  //   }
  // }
  
  this->m_KeepAlive = this->m_Response.isKeepAlive();
}
