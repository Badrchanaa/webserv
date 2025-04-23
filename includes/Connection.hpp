#ifndef __Connection__
#define __Connection__

#include "./WebServer.hpp"

class Connection {

  public:
    typedef enum {
      REQUEST_PARSING,
      RESPONSE_PROCESSING,
      RESPONSE_SENT,
    } connectionState;


  public:
    Connection(int f, struct sockaddr_storage a): fd(f), state(REQUEST_PARSING), addr(a) {}
    FileDescriptor fd;

    void  reset()
    {
      this->m_State = RESPONSE_PROCESSING;
      this->m_Request.reset();
      this->m_Response.reset();
    }

    bool  keepAlive()
    {
      return m_KeepAlive;
    }

    void  init_response() {
      this->m_State = RESPONSE_PROCESSING;
      this->response.init(this->m_Request);
      this->m_KeepAlive = request.isKeepAlive();
    }
  //struct sockaddr_storage addr; //what is this

  private:
    bool      m_KeepAlive;
    HTTPRequest m_Request;
    HTTPResponse m_Response;
    connectionState m_State;

};

#endif
