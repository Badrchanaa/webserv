#ifndef __Connection__
#define __Connection__

#include "./WebServer.hpp"

class Connection {
public:
  typedef enum {
    REQUEST_PARSING,
    RESPONSE_PROCESSING,
  } connectionState;

public:
  FileDescriptor fd;
  // std::string request;
  HTTPRequest request;
  HTTPResponse response;
  connectionState state;

  void init_response(HTTPRequest &request) {
    this->state = RESPONSE;
    this->response.init(request, handler);
  }
  struct sockaddr_storage addr;

  Connection(int f, struct sockaddr_storage a)
      : fd(f), state(REQUEST), addr(a) {}
};

#endif
