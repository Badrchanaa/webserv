#include "./cgi.hpp"
bool CGIHandler::is_cgi_socket(int fd) const {
  return processes.find(fd) != processes.end();
}
// CGIHandler::CGIHandler(int efd) : epoll_fd(efd) {}
// /path/tocgi/script, env, clientfd, requestBody
void CGIHandler::spawn(const std::string &script,
                       const std::vector<std::string> &env, int client,
                       const std::string &body) {
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
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
    ev.data.fd = sockets[0];
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets[0], &ev);

    CGIProcess proc = {client, sockets[0], -1,  pid, "", body, 0};
    processes[sockets[0]] = proc;
  }
}

void CGIHandler::handle_cgi_request(int fd, uint32_t events) {
  std::map<int, CGIProcess>::iterator it = processes.find(fd);
  if (it == processes.end())
    return;

  if (events & EPOLLOUT)
    handle_output(it->second);
  if (events & EPOLLIN)
    handle_input(it->second);
  if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    cleanup(it->second, true);
}

void CGIHandler::check_zombies() {
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

void CGIHandler::setup_child(int sock, int open_fd, const std::string &script,
                             const std::vector<std::string> &env) {
  if (open_fd != -1)
    dup2(open_fd ,STDIN_FILENO);
  else
    dup2(sock, STDIN_FILENO);
  dup2(sock, STDOUT_FILENO);
  close(sock);
  close(open_fd);

  std::vector<const char *> envp;
  for (size_t i = 0; i < env.size(); ++i)
    envp.push_back(env[i].c_str());
  envp.push_back(NULL);

  const char *argv[] = {script.c_str(), NULL};
  execve(script.c_str(), (char *const *)argv, (char *const *)&envp[0]);
  exit(EXIT_FAILURE);
}

void CGIHandler::handle_output(CGIProcess &proc) {
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

void CGIHandler::handle_input(CGIProcess &proc) {
  char buf[4096];
  ssize_t received = recv(proc.cgi_sock, buf, sizeof(buf), 0);

  if (received > 0) {
    proc.output.append(buf, received);
  } else if (received <= 0 && errno != EAGAIN) {
    cleanup(proc, received < 0);
  }
}

void CGIHandler::cleanup(CGIProcess &proc, bool error) {
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

