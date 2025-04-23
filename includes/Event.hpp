#ifndef __Event__
#define __Event__

#include "WebServer.hpp"

typedef enum { RESPONSE_EVENT, REQUEST_EVENT, CLIENT_EVENT } eventType;

class Event {
public:
  int fd;
  int flags;
  eventType type;
  void *listener;
};



#endif // !DEBUG
