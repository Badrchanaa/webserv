#include "Connection.hpp"


// Connection::Connection(CGIHandler &cgihandler, std::vector<ConfigServer> &servers, Config &conf, int f)
//       : m_State(REQUEST_PARSING), config(conf), Cgihandler(cgihandler), client_fd(f), m_Request(servers), hasEvent(false), cgiEvent(false) , socketEvent(false), events(false) {

//       std::cout << "hello test test" << std::endl;
//   }
Connection::Connection(CGIHandler &cgihandler, std::vector<ConfigServer> &servers, int f)
      : m_State(REQUEST_PARSING), Cgihandler(cgihandler), client_fd(f), m_Request(servers), hasEvent(false), cgiEvent(false) , socketEvent(false), events(0) {

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

  void Connection::init_response() {
    this->m_State = RESPONSE_PROCESSING;
    this->m_Response.init(this->m_Request, this->Cgihandler, m_Request.getServer(),
                          this->client_fd);
    this->m_KeepAlive = this->m_Response.isKeepAlive();
  }
