#include "../../includes/CGIHandler.hpp"
#include "../../includes/WebServer.hpp"

void CGIHandler::cleanup_by_fd(int fd) {
  std::map<int, CGIProcess>::iterator it = processes.find(fd);
  if (it != processes.end()) {
    cleanup(it->second, true);
  }
}


int CGIHandler::getCgiSocket(int c_fd) const {
    const std::map<int, CGIProcess>& processes = this->processes;
    for (std::map<int, CGIProcess>::const_iterator it = processes.begin(); 
         it != processes.end(); ++it) {
        if (it->second.client_fd == c_fd) {
            return it->first;
        }
    }
    return -1;
}

// bool CGIHandler::is_cgi_socket(int fd) const {
//   return processes.find(fd) != processes.end();
// }

int CGIHandler::is_cgi_socket(int fd) const {
  std::map<int, CGIProcess>::const_iterator it = processes.find(fd);
  if (it != processes.end())
    return it->second.client_fd;
  return false; // Not found
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
    // ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
    ev.events = EPOLL_READ | EPOLL_WRITE; 
    ev.data.fd = sockets[0];
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets[0], &ev);

    CGIProcess proc = {client, sockets[0], pid, "", body, 0};
    processes[sockets[0]] = proc;
  }
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

//     for (std::map<int, CGIProcess>::iterator it = processes.begin();
//          it != processes.end(); ++it) {
//       if (it->second.pid == pid) {
//         cleanup(it->second, WIFEXITED(status) ? false : true);
//         break;
//       }
//     }
//   }
// }

void CGIHandler::setup_child(int sock, const std::string &script,
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
  exit(EXIT_FAILURE);
}
//
// bool  CGIProcess::handle_input(HTTPBody& body) {
//   // Hello World + 5 >> World
//   const char * buff = body.getBuffer();
//   size_t len = body.getSize();
//   ssize_t sent = send(this->cgi_sock, buff + this->written,
//                       len - this->written, MSG_NOSIGNAL);
//   if(sent < 0) {
//     this->cleanup(*this, true);
//     return false;
//   }
//   body.setOffset(sent);
//   return true;
//   // if (sent < len) {
//   //     struct epoll_event ev;
//   //     ev.events = EPOLL_READ;
//   //     ev.data.fd = this->cgi_sock;
//   //     epoll_ctl(epoll_fd, EPOLL_CTL_MOD, proc.cgi_sock, &ev);
//   //     shutdown(proc.cgi_sock, SHUT_WR);
//   // }
// }

// void CGIProcess::handle_output(CGIProcess &proc) {
//   char buf[4096];
//   ssize_t received = recv(this->cgi_sock, buf, sizeof(buf), 0);

//   if (received > 0) {
//     proc.output.append(buf, received);
//   } else if (received <= 0 && errno != EAGAIN) {
//     this->cleanup(*this, received < 0);
//   }
// }

// void CGIHandler::cleanup(bool error) {
void CGIHandler::cleanup(const CGIProcess &proc, bool error){
  // if (!proc.output.empty()) {
  //   send(proc.client_fd, proc.output.c_str(), proc.output.size(), 0);
  // } else if (error) {
  //   // std::string err = "HTTP/1.1 500 Internal Error\r\n\r\n";
  //   send(proc.client_fd, err.c_str(), err.size(), 0);
  // }
  (void)error;
  close(proc.cgi_sock);
  // close(proc.client_fd);
  processes.erase(proc.cgi_sock);
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
