
#ifndef __EpollManager__
#define __EpollManager__

#include "./WebServer.hpp"

class EpollManager {
  std::map<int, Event> events;
public:
  int epfd;

  EpollManager();
  ~EpollManager();

  void mod_fd(int fd, uint32_t events);
  void remove_fd(int fd);
  void add_fd(int fd, uint32_t events);
private:
  std::string format_events(uint32_t events);
};

#endif
