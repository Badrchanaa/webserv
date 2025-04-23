
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
  int subscribe(int fd, uint32_t flags, int type, void *listener);
  void wait();
  void unregister(int fd);
  void notify(Event &event, Connection &connection);

private:
  std::string format_events(uint32_t events);
};

#endif
