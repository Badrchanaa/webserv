
#ifndef __EpollManager__
#define __EpollManager__

// #include "WebServer.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream> 
#include <ctime> 
#include <iostream>
#include <list>
#include <map>
#include <netdb.h>
#include <csignal>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
class EpollManager {
  // std::map<int, Event> events;
public:
  int epfd;
  // // typedef  uint32_t events_t;
  // typedef std::map<int,events_t> eventsMap;
  // eventsMap fds;

  EpollManager();
  ~EpollManager();

  void mod_fd(int fd, uint32_t events);
  void remove_fd(bool &is_added, int fd);
  void add_fd(bool &is_added, int fd, uint32_t events);
// private:
  std::string format_events(uint32_t events);
};

#endif
