#include "Connection.hpp"

// Connection::Connection(CGIHandler &cgihandler, std::vector<ConfigServer> &servers, Config &conf, int f)
//       : m_State(REQUEST_PARSING), config(conf), Cgihandler(cgihandler), client_fd(f), m_Request(servers), hasEvent(false), cgiEvent(false) , socketEvent(false), events(false) {

//       std::cout << "hello test test" << std::endl;
//   }
Connection::Connection(std::vector<ConfigServer> &servers, int f)
      : cgiFdAdded(false) , socketFdAdded(false), m_State(REQUEST_PARSING), client_fd(f), m_Request(servers), hasEvent(false), cgiEvent(false) , socketEvent(false), events(0) {

  }
  // Connection(CGIHandler &cgihandler, Config &conf, int f,
  //            struct sockaddr_storage a)
  // : client_fd(f), state(REQUEST_PARSING), addr(a) {}

  void Connection::reset() {
    this->m_State = RESPONSE_PROCESSING;
    this->m_Request.reset();
    this->m_Response.reset();
  }

void Connection::resetEvents()
{
  this->hasEvent = false;
  this->socketEvent = false;
  this->cgiEvent = false;
  this->events = 0;
}



  bool Connection::keepAlive() { return this->m_KeepAlive; }

// asfs
  void Connection::init_response(EpollManager& epollManager, CGIHandler& cgiHandler) {
    this->m_State = RESPONSE_PROCESSING;
  // if cgi init will spawn and setup the child and wait for cgi_write to write body;
    this->m_Response.init(this->m_Request, cgiHandler, m_Request.getServer(),
                          this->client_fd);
    HTTPResponse::pollState state = m_Response.getPollState();
    if ( state == HTTPResponse::CGI_READ || 
        state == HTTPResponse::CGI_WRITE)
    {
      epollManager.remove_fd(this->client_fd);
      std::cout << "----------------------------+" << std::endl;
      epollManager.add_fd(m_Response.getCgiFd(), EPOLL_WRITE | EPOLL_READ);
      std::cout << "----------------------------+" << std::endl;
      DEBUG_LOG("Switched to monitoring CGI socket.");
    }
    // else
    //   epollManager.add_fd(this->client_fd, EPOLL_READ | EPOLL_WRITE);
    // add
    this->m_KeepAlive = this->m_Response.isKeepAlive();
  }
