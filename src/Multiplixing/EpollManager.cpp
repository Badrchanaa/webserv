
#include "./../../includes/EpollManager.hpp"

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

  void EpollManager::remove_fd(int fd) {
    DEBUG_LOG("[Epoll] Attempting to remove fd: " << fd);
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
      DEBUG_LOG("[Epoll] Remove failed (fd: " << fd
                                              << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl del failed");
    }
    DEBUG_LOG("[Epoll] Successfully removed fd: " << fd);
  }

  void EpollManager::add_fd(int fd, uint32_t events) {
    DEBUG_LOG("[Epoll] Adding fd "
              << fd << " with events: " << format_events(events));
    struct epoll_event ev;
    ev.events = events | EPOLL_ERRORS;
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      DEBUG_LOG("[Epoll] Add failed (fd: " << fd << "): " << strerror(errno));
      throw std::runtime_error("epoll_ctl add failed");
    }
    DEBUG_LOG("[Epoll] Successfully added fd " << fd);
  }

  int EpollManager::subscribe(int fd, uint32_t flags, int type, void *listener) {
    DEBUG_LOG("[Epoll] Adding fd "
              << fd << " with events: " << format_events(events));
    std::map<int, Event>::iterator it = events.find(fd);
    if (it != events.end())
      return 1;

    events[fd] = {fd, flags, type, listener};
    this->add_fd(fd, flags | EPOLL_ERRORS);
    // this->add_mod(fd, flags | EPOLL_ERRORS);
  }

  void EpollManager::wait() {
    epoll_wait();
    for (event in epoll_events) {
      if (events.(event.fd)) {
        notify(event);
      }
    }
  }

  void EpollManager::unregister(int fd) {
    // delete from map and epoll context
  }

  void EpollManager::notify(Event &event, Connection &connection) {
    if (event.type == REQUEST) {
      connection.handle_request();
      connection.handle_response();
      bool should_delete = request->resume(event.fd);
    } else {
      HTTPResponse *response = static_cast<HTTPResponse *>(event.listener);
      bool should_delete = response->resume(event.fd);
    }
    if (should_delete) {
      epoll_ctl DELETE;
    }
    // event.listener
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
