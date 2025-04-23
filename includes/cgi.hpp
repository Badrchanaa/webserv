#ifndef __cgi__
#define __cgi__

// #include "./test.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <fstream> // Required for file operations
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <map>
#include <netdb.h>
#include <signal.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <map>
#include <netdb.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

// CGIHandler

// Response.CGIHandler

#include <map>
#include <vector>

class Manager {

  // struct epoll_event ev;
    // ev.events = events;
    // ev.data.fd = fd;
  std::map<int, Event> events;

public:
  int subscribe(int fd, int type, void *listener, int flags)
  {
    // Event event;
    // event.listener = listener;
    std::map<int, Event>::iterator it = events.find(fd);
    if (it != events.end())
      return 1;
    


    epoll_ctl(instance, EPOLL_CTL_ADD, fd, ev);
  }

  void remove()
  {
      events.delete(fd, event);
  }

  void loop()
  {
    epoll_wait();
    for(event in events)
    {
      event.http_instance.resume();
      if (event.flags | REMOVE_ON_EVENT)
        remove(fd, event);
    }
  }
};

class CGIHandler {
private:
  struct  CGIProcess {
    // int  
    int client_fd;
    int cgi_sock;
    pid_t pid;
    std::string output;
    std::string input;
    size_t written;
  };

  std::map<int, CGIProcess> processes;

public:
  int epoll_fd;
  // CGIHandler(int efd);
  CGIHandler(int ep_fd = -1) : epoll_fd(ep_fd) {}
  // explicit  CGIHandler(int ep_fd = 0) : epoll_fd(ep_fd) {}
  bool is_cgi_socket(int fd) const;
  void spawn(const std::string &script, const std::vector<std::string> &env, int client, const std::string &body);
  void handle_cgi_request(int fd, uint32_t events);
  void check_zombies();
  void setup_child(int sock, const std::string &script, const std::vector<std::string> &env);
  void handle_output(CGIProcess &proc);
  void handle_input(CGIProcess &proc);
  //read(index.html);
  void cleanup(CGIProcess &proc, bool error);
};


#endif // !__cgi__
