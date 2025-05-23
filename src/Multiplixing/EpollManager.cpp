#include "../../includes/EpollManager.hpp"
#include "../../includes/WebServer.hpp"
// # define DEBUG_LOG(name) name

EpollManager::EpollManager() {
    DEBUG_LOG("[Epoll] Creating epoll instance");
    epfd = epoll_create(1337);
    if (epfd == -1) {
      DEBUG_LOG("[Epoll] Creation failed: " << strerror(errno));
      throw std::runtime_error("epoll_create failed");
    }
    DEBUG_LOG("[Epoll] Created successfully (fd: " << epfd << ")");
  }

  EpollManager::~EpollManager() {
    DEBUG_LOG("[Epoll] Destroying epoll instance (fd: " << epfd << ")");
    if (epfd != -1) {
      if (close(epfd)) {
        DEBUG_LOG("[Epoll] Close failed: " << strerror(errno));
      }
    }
  }

  void EpollManager::mod_fd(int fd, uint32_t events) {
    DEBUG_LOG("[Epoll] Modifying fd "
              << fd << " with events: " << format_events(events));
    struct epoll_event ev;
    ev.events = events | EPOLL_ERRORS;

    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
      DEBUG_LOG("[Epoll] Modify failed (fd: " << fd
                                              << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl mod failed");
    }
    DEBUG_LOG("[Epoll] Successfully modified fd " << fd);
  }

  void EpollManager::remove_fd(bool &is_added, int fd) {
    
    if (fd <= 0)
    {
      DEBUG_LOG("[Epoll] remove_fd fd is less than 0 :  " << fd);
      return ;
    }
    DEBUG_LOG("[Epoll] Attempting to remove fd: " << fd);
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
      DEBUG_LOG("[Epoll] Remove failed (fd: " << fd
                                              << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl del failed");
    }
    is_added = false;
    DEBUG_LOG("[Epoll] Successfully removed fd: " << fd);
  }

  void EpollManager::add_fd(bool &is_added, int fd, uint32_t events) {
    // eventsMap::const_iterator it = fds.find(fd);
    // if (it == fds.end())
    //   return ;
    DEBUG_LOG("[Epoll] Adding fd "
              << fd << " with events: " << format_events(events));
    struct epoll_event ev;
    ev.events = events | EPOLL_ERRORS;
    // ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      DEBUG_LOG("[Epoll] Add failed (fd: " << fd << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl add failed");
    }
    is_added = true;
    DEBUG_LOG("[Epoll] Successfully added fd " << fd);
  }


  std::string EpollManager::format_events(uint32_t events) {
    std::string result;
    if (events & EPOLLIN)
      result += "EPOLLIN|";
    if (events & EPOLLOUT)
      result += "EPOLLOUT|";
    if (events & EPOLLRDHUP)
      result += "EPOLLRDHUP|";
    if (events & EPOLLET)
      result += "EPOLLET|";
    if (events & EPOLLONESHOT)
      result += "EPOLLONESHOT|";
    if (events & EPOLLERR)
      result += "EPOLLERR|";
    if (events & EPOLLHUP)
      result += "EPOLLHUP|";

    return result.empty() ? "NONE" : result;
  }
