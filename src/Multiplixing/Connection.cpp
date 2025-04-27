#include "../../includes/Connection.hpp"


Connection::Connection(CGIHandler &cgihandler, ServerConfig &server, Config &conf, int f)
      : config(conf), Cgihandler(cgihandler), server_config(conf), client_fd(f), hasEvent(false), cgiEvent(false) , socketEvent(false), events(false), m_State(REQUEST_PARSING) {}
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
  this->events = false;
}



  bool Connection::keepAlive() { return this->m_KeepAlive; }

  void Connection::init_response() {
    this->m_State = RESPONSE_PROCESSING;
    this->m_Response.init(this->m_Request, this->Cgihandler, this->config,
                          this->client_fd);
    this->m_KeepAlive = this->m_Response.isKeepAlive();
  }
