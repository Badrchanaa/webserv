#include "../../includes/CGIHandler.hpp"
#include "../../includes/WebServer.hpp"

CGIProcess *CGIHandler::spawn(std::string &pathName, std::string &scriptName, char **env) const {
  CGIProcess *proc = NULL;
  int sockets[2];
  std::cout << "SOCKET PAIR CALLED" << std::endl;
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets))
    throw std::runtime_error("socketpair");

  pid_t pid = fork();
  if (pid < 0) {
    close(sockets[0]);
    close(sockets[1]);
    throw std::runtime_error("fork");
  }

  if (pid == 0) {
    close(sockets[0]);
    // setup_child(sockets[1], pathName, scriptName, environ);
    setup_child(sockets[1], pathName, scriptName, env);
  }
  close(sockets[1]);

  std::cout << "socket pair : ----------------------------------------------> "
            << sockets[0] << std::endl;
  proc = new CGIProcess(sockets[0], pid);
  return proc;
}


void CGIHandler::setup_child(int sock, std::string &pathName, std::string &scriptName, char **env) const
{
  sighandler_t handler;

  handler = SIG_IGN;
  signal(SIGINT, handler);
  if (dup2(sock, STDIN_FILENO) == -1 || dup2(sock, STDOUT_FILENO) == -1) {
      std::cerr << "dup2 failed: " << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
  }

  std::string script_path(scriptName);
  size_t last_slash = scriptName.find_last_of('/');
  std::string script_dir = (last_slash != std::string::npos) ? scriptName.substr(0, last_slash) : ".";
  scriptName = (last_slash != std::string::npos) ? scriptName.substr(last_slash + 1) : "";

  
  if (chdir(script_dir.c_str()) == -1) {
      std::cerr << "chdir failed: " << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
  }
  char *const args[3] = {const_cast<char *const>(pathName.c_str()),
                        const_cast<char *const>(scriptName.c_str()), NULL};

  // std::cout << "test " << sock << std::endl;
  // std::cout << "args[0] :: " << args[0] << std::endl;
  // std::cout << "args[1] :: " << args[1] << std::endl;

  // setenv("SCRIPT_FILENAME", "/path/to/script.php", 1);
  // setenv("PHP_SELF", "app/cgi-bin/script.php", 1);
  // setenv("SERVER_SOFTWARE", "WEBSERV/1.0", 1);

    (void)env;
  // execve(args[0], args, env);
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

  if (received < 0) { // -1
    std::cout << "++++++++++++++++++++++++++++" << std::endl;
    std::cout << "received == < 0" << std::endl;
    this->cleanup(true);
    std::cout << "++++++++++++++++++++++++++++" << std::endl;
  } else if (received == 0) {
    std::cout << "----------------------------" << std::endl;
    std::cout << "received == 0" << std::endl;
    this->cleanup(false);
    std::cout << "----------------------------" << std::endl;
  }
  std::cout << "RECEIVED: " << received << std::endl;
  return received;
}

// In CGIHandler
void CGIProcess::cleanup(bool error) {

  if (error) {
    kill(pid, SIGKILL);
    std::cerr << "[CGI] Forcefully terminated PID " << pid << std::endl;
  }

  int status;
  if (waitpid(pid, &status, 0) == 0) {
    // Process still running, wait normally
    // waitpid(pid, &status, 0);
  }

  if (cgi_sock != -1) {
    // close(cgi_sock);
    cgi_sock = -1;
  }

  if (WIFEXITED(status)) {
    std::cout << "[CGI] Process " << pid << " exited with status "
              << WEXITSTATUS(status) << std::endl;
  }
}
