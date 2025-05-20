#include "../../includes/CGIHandler.hpp"
#include "../../includes/WebServer.hpp"
#include <cstdlib>
// /path/tocgi/script, env, clientfd, requestBody
extern char **environ;
CGIProcess *CGIHandler::spawn(char *const *args) const {
  CGIProcess *proc = NULL;
  int sockets[2];
  std::cout << "SOCKET PAIR CALLED" << std::endl;
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets))
    throw std::runtime_error("socketpair");

  // struct stat sb;
  // if (stat(args[0], &sb) != 0 || !S_ISREG(sb.st_mode) ||
  //     access(args[0], X_OK) != 0) {
  //   close(sockets[0]);
  //   close(sockets[1]);
  //   throw std::runtime_error("Invalid CGI script: " + std::string(args[0]));
  // }
  pid_t pid = fork();
  if (pid < 0) {
    close(sockets[0]);
    close(sockets[1]);
    throw std::runtime_error("fork");
  }

  if (pid == 0) {

    close(sockets[0]);
    // std::cout << "aargs[0] :: " << args[0] << std::endl;
    // std::cout << "aargs[1] :: " << args[1] << std::endl;
    setup_child(sockets[1], args, environ);
    // exit(1);
  }
  close(sockets[1]);

  std::cout << "socket pair : ----------------------------------------------> "
            << sockets[0] << std::endl;
  proc = new CGIProcess(sockets[0], pid);
  return proc;
}

void CGIHandler::setup_child(int sock, char *const *args, char **env) const {
  // for (int fd = sysconf(_SC_OPEN_MAX); fd > 2; fd--) {
  //   if (fd != sock)
  //     close(fd);
  // }
  dup2(sock, STDERR_FILENO); // Add this line
  dup2(sock, STDIN_FILENO);
  dup2(sock, STDOUT_FILENO);
  close(sock);

  (void)env;
  std::cout << "test " << std::endl;
  std::cout << "args[0] :: " << args[0] << std::endl;
  std::cout << "args[1] :: " << args[1] << std::endl;

  execve(args[0], args, environ);
  std::cerr << "execve failed: " << strerror(errno) << std::endl;
  exit(EXIT_FAILURE);
}
//
bool CGIProcess::write(HTTPBody &body) {
  // Hello World + 5 >> World
  const char *buff = body.getBuffer();
  size_t len = body.getSize();
  std::cout << "buff : " << buff << std::endl;
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
