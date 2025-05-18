#include "../../includes/CGIHandler.hpp"
#include "../../includes/WebServer.hpp"

// void CGIHandler::cleanup_by_fd(int fd) {
//   std::map<int, CGIProcess>::iterator it = processes.find(fd);
//   if (it != processes.end()) {
//     cleanup(it->second, true);
//   }
// }

// int CGIHandler::getCgiSocket(int c_fd) const {
//   const std::map<int, CGIProcess> &processes = this->processes;
//   for (std::map<int, CGIProcess>::const_iterator it = processes.begin();
//        it != processes.end(); ++it) {
//     if (it->second.client_fd == c_fd) {
//       return it->first;
//     }
//   }
//   return -1;
// }

// bool CGIHandler::is_cgi_socket(int fd) const {
//   return processes.find(fd) != processes.end();
// }

// int CGIHandler::is_cgi_socket(int fd) const {
//   std::map<int, CGIProcess>::const_iterator it = processes.find(fd);
//   if (it != processes.end())
//     return it->second.client_fd;
//   return false; // Not found
// }

// CGIHandler::CGIHandler(int efd) : epoll_fd(efd) {}
// /path/tocgi/script, env, clientfd, requestBody
extern char **environ;
CGIProcess *CGIHandler::spawn(char * const *args) const
{
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
    setup_child(sockets[1], args, environ);
  } else {
    close(sockets[1]);

    // struct epoll_event ev;
    // ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
    // ev.events = EPOLL_READ | EPOLL_WRITE;
    // ev.data.fd = sockets[0];
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets[0], &ev);
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets[0], &ev);

    // mod_fd(socket
    std::cout << "socket pair : ----------------------------------------------> " << sockets[0] << std::endl;
    proc = new CGIProcess(sockets[0], pid);
  }
  return proc;
}

// void CGIHandler::handle_cgi_request(int fd, uint32_t events) {
//   std::map<int, CGIProcess>::iterator it = processes.find(fd);
//   if (it == processes.end())
//     return;
//
//   if (events & EPOLL_WRITE)
//     handle_output(it->second);
//   if (events & EPOLL_READ)
//     handle_input(it->second);
//   if (events & (EPOLL_ERRORS))
//     cleanup(it->second, true);
// }

// void CGIHandler::check_zombies() {
//   int status;
//   pid_t pid;
//   while ((pid = waitpid(-1, &status, WNOHANG))) {
//     if (pid <= 0)
//       break;
//
//     for (std::map<int, CGIProcess>::iterator it = processes.begin();
//          it != processes.end(); ++it) {
//       if (it->second.pid == pid) {
//         cleanup(it->second, WIFEXITED(status) ? false : true);
//         break;
//       }
//     }
//   }
// }

void CGIHandler::setup_child(int sock, char * const* args, char **env) const {
  dup2(sock, STDIN_FILENO);
  dup2(sock, STDOUT_FILENO);
  close(sock);

  // std::vector<const char *> envp;
  // for (size_t i = 0; env[i]; ++i)
  //   envp.push_back(env[i].c_str());
  // envp.push_back(NULL);

  if (execve(args[0], args, env) == -1)
    exit(EXIT_FAILURE);
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
  std::cout << "received: " << received  << " from: " << cgi_sock << std::endl;
  // proc.output.append(buf, received);
  if (received <= 0) {
    this->cleanup(received < 0);
    std::cout << "cgi read error: " << strerror(errno) << std::endl;
    // this->cleanup(*this, received < 0);
  }
  return received;
}

// void CGIHandler::cleanup(bool error) {
void CGIProcess::cleanup(bool error) {
  (void)error;
  if (error)
    kill(this->pid, SIGKILL);
  // close(this->cgi_sock);
  // close(proc.client_fd);
  // processes.erase(proc.cgi_sock);
  // int status;
  // waitpid(pid, NULL, WNOHANG);
  waitpid(pid, NULL, 0);
}

// void handle_request(Connection &conn) {
//   if (needs_cgi(conn.request)) {
//     std::map<std::string, std::string> env;
//     // Populate environment variables...

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
