#include "../../includes/CGIHandler.hpp"
#include "../../includes/WebServer.hpp"
// /path/tocgi/script, env, clientfd, requestBody
extern char **environ;
CGIProcess *CGIHandler::spawn(char *const *args) const {
  CGIProcess *proc = NULL;
  int sockets[2];
  std::cout << "SOCKET PAIR CALLED" << std::endl;
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets))
    throw std::runtime_error("socketpair");

  pid_t pid = fork();
  if (pid < 0)
    throw std::runtime_error("fork");

  if (pid == 0) {

    close(sockets[0]);
    // std::cout << "aargs[0] :: " << args[0] << std::endl;
    // std::cout << "aargs[1] :: " << args[1] << std::endl;
    setup_child(sockets[1], args, environ);
  } else {
    close(sockets[1]);

    // struct epoll_event ev;
    // ev.events = EPOLL_READ | EPOLL_WRITE;
    // ev.data.fd = sockets[0];
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets[0], &ev);

    std::cout
        << "socket pair : ----------------------------------------------> "
        << sockets[0] << std::endl;
    proc = new CGIProcess(sockets[0], pid);
  }
  return proc;
}

void CGIHandler::setup_child(int sock, char *const *args, char **env) const {
  dup2(sock, STDIN_FILENO);
  dup2(sock, STDOUT_FILENO);
  close(sock);

  std::cout << "test " << std::endl;
  // std::cout << "args[0] :: " << args[0] << std::endl;
  // std::cout << "args[1] :: " << args[1] << std::endl;

  if (execve(args[0], args, env) == -1) {
    std::cerr << "execve failed: " << strerror(errno) << std::endl;
    exit(EXIT_FAILURE);
  }
}
//
bool CGIProcess::write(HTTPBody &body) {
  // Hello World + 5 >> World
  const char *buff = body.getBuffer();
  size_t len = body.getSize();
  ssize_t sent = send(this->cgi_sock, buff, len, MSG_NOSIGNAL);
  if (sent < 0) {
    this->cleanup(true);
    return false;
  }
  body.setOffset(sent);
  return true;
}

ssize_t CGIProcess::read(char *buff, size_t size) {
  ssize_t received = recv(this->cgi_sock, buff, size, 0);
  std::cout << "received: " << received << " from: " << cgi_sock << std::endl;

  if (received < 0) {
    std::cout << "++++++++++++++++++++++++++++" << std::endl;
    std::cout << "received == < 0" << std::endl;
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0; // Not a fatal error for non-blocking sockets
    }
    this->cleanup(true);
    std::cout << "++++++++++++++++++++++++++++" << std::endl;
  } else if (received == 0) {
    std::cout << "----------------------------" << std::endl;
    std::cout << "received == 0" << std::endl;
    this->cleanup(false);
    std::cout << "----------------------------" << std::endl;
  }
  return received;
}

// In CGIHandler
void CGIProcess::cleanup(bool error) {
  if (cgi_sock != -1) {
    close(cgi_sock);
    cgi_sock = -1;
  }

  if (error) {
    kill(pid, SIGKILL);
    std::cerr << "[CGI] Forcefully terminated PID " << pid << std::endl;
  }

  int status;
  if (waitpid(pid, &status, WNOHANG) == 0) {
    // Process still running, wait normally
    waitpid(pid, &status, 0);
  }

  if (WIFEXITED(status)) {
    std::cout << "[CGI] Process " << pid << " exited with status "
              << WEXITSTATUS(status) << std::endl;
  }
}
