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

class CGIHandler {
  struct CGIProcess {
    int client_fd;
    int cgi_sock;
    pid_t pid;
    std::string output;
    std::string input;
    size_t written;
  };

  std::map<int, CGIProcess> processes;
  int epoll_fd;

public:
  bool is_cgi_socket(int fd) const {
    return processes.find(fd) != processes.end();
  }
  CGIHandler(int efd) : epoll_fd(efd) {}

  void spawn(const std::string &script, const std::vector<std::string> &env,
             int client, const std::string &body) {
    int sockets[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets))
      throw std::runtime_error("socketpair");

    pid_t pid = fork();
    if (pid < 0)
      throw std::runtime_error("fork");

    if (pid == 0) {
      close(sockets[0]);
      setup_child(sockets[1], script, env);
    } else {
      close(sockets[1]);

      struct epoll_event ev;
      ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
      ev.data.fd = sockets[0];
      epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets[0], &ev);

      CGIProcess proc = {client, sockets[0], pid, "", body, 0};
      processes[sockets[0]] = proc;
    }
  }

  void handle(int fd, uint32_t events) {
    std::map<int, CGIProcess>::iterator it = processes.find(fd);
    if (it == processes.end())
      return;

    if (events & EPOLLOUT)
      handle_output(it->second);
    if (events & EPOLLIN)
      handle_input(it->second);
    if (events & (EPOLLERR | EPOLLHUP))
      cleanup(it->second, true);
  }

  void check_zombies() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG))) {
      if (pid <= 0)
        break;

      for (std::map<int, CGIProcess>::iterator it = processes.begin();
           it != processes.end(); ++it) {
        if (it->second.pid == pid) {
          cleanup(it->second, WIFEXITED(status) ? false : true);
          break;
        }
      }
    }
  }

private:
  void setup_child(int sock, const std::string &script,
                   const std::vector<std::string> &env) {
    dup2(sock, STDIN_FILENO);
    dup2(sock, STDOUT_FILENO);
    close(sock);

    std::vector<const char *> envp;
    for (size_t i = 0; i < env.size(); ++i)
      envp.push_back(env[i].c_str());
    envp.push_back(NULL);

    const char *argv[] = {script.c_str(), NULL};
    execve(script.c_str(), (char *const *)argv, (char *const *)&envp[0]);
    _exit(EXIT_FAILURE);
  }

  void handle_output(CGIProcess &proc) {
    // Hello World + 5 >> World
    ssize_t sent = send(proc.cgi_sock, proc.input.c_str() + proc.written,
                        proc.input.size() - proc.written, MSG_NOSIGNAL);

    if (sent > 0) {
      proc.written += sent;
      if (proc.written >= proc.input.size()) {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = proc.cgi_sock;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, proc.cgi_sock, &ev);
        shutdown(proc.cgi_sock, SHUT_WR);
      }
    } else if (sent < 0 && errno != EAGAIN) {
      cleanup(proc, true);
    }
  }

  void handle_input(CGIProcess &proc) {
    char buf[4096];
    ssize_t received = recv(proc.cgi_sock, buf, sizeof(buf), 0);

    if (received > 0) {
      proc.output.append(buf, received);
    } else if (received <= 0 && errno != EAGAIN) {
      cleanup(proc, received < 0);
    }
  }

  void cleanup(CGIProcess &proc, bool error) {
    if (!proc.output.empty()) {
      send(proc.client_fd, proc.output.c_str(), proc.output.size(), 0);
    } else if (error) {
      std::string err = "HTTP/1.1 500 Internal Error\r\n\r\n";
      send(proc.client_fd, err.c_str(), err.size(), 0);
    }

    close(proc.cgi_sock);
    close(proc.client_fd);
    processes.erase(proc.cgi_sock);
  }
};

// void handle_request(Connection &conn) {
//   if (needs_cgi(conn.request)) {
//     std::map<std::string, std::string> env;
//     // Populate environment variables...
//
//     try {
//       cgi_handler.spawn("/path/to/cgi/script", env, conn.client_fd,
//                         conn.request.body);
//     } catch (const std::exception &e) {
//       send_error_response(conn.client_fd, 500);
//     }
//   } else {
//     // Handle normal request
//   }
// }
// In your epoll event handling loop:
// for (int i = 0; i < num_events; ++i) {
//   int current_fd = events[i].data.fd;
//
//   if (cgi_handler.is_cgi_socket(current_fd)) {
//     // Handle CGI socket I/O
//     cgi_handler.handle(current_fd, events[i].events);
//   } else {
//     // Handle regular client socket
//     handle_client(current_fd, events[i].events); }
// }
// In your main event loop:
//
//
//

void handle_cgi_request(Connection &conn, const RouteConfig &route,
                        EpollManager &epoll) {
  // Prepare environment variables
  std::map<std::string, std::string> env;
  env["REQUEST_METHOD"] = conn.request.method;
  env["SCRIPT_FILENAME"] = route.resolve_path(conn.request.uri);
  env["QUERY_STRING"] = conn.request.query;
  env["CONTENT_LENGTH"] = toString(conn.request.body.size());
  env["SERVER_PROTOCOL"] = "HTTP/1.1";
  // ... add other required variables ...

  // Initialize CGI handler
  conn.cgi_handler = new CGIHandler(epoll);

  try {
    conn.cgi_handler->spawn(route.cgi_path, env, conn.fd, conn.request.body);

    // Modify epoll to monitor CGI socket
    epoll.modify_fd(conn.fd, EPOLLOUT); // Prepare to send request body
  } catch (const std::exception &e) {
    send_error_response(conn, 502);
  }
}

void handle_request(Connection *conn, const Config &config,
                    EpollManager &epoll) {
  try {

    if (route->cgi_enabled && is_cgi_request(conn->request.uri, route)) {
      handle_cgi_request(*conn, *route, epoll);
      return;
    }
  }

  void test() {
    CGIHandler cgi(epoll_fd);

    while (1) {
      int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
      cgi.check_zombies();

      for (int i = 0; i < n; ++i) {
        if (cgi.is_cgi_socket(events[i].data.fd)) {
          cgi.handle(events[i].data.fd, events[i].events);
        } else {
          handle_client(current_fd, events[i].events);
          // Handle normal connections
        }
      }
    }
  }
