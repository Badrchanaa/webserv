#include "../../includes/CGIHandler.hpp"
#include "../../includes/WebServer.hpp"

CGIProcess *CGIHandler::spawn(std::string &pathName, std::string &scriptName, char **env) const {
  CGIProcess *proc = NULL;
  int stdout_sockets[2];
  int stderr_sockets[2];
  
  std::cout << "SOCKET PAIR CALLED" << std::endl;
  
  // socket pair for stdout
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, stdout_sockets))
    throw std::runtime_error("socketpair for stdout");
    
  // socket pair for stderr
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, stderr_sockets)) {
    close(stdout_sockets[0]);
    close(stdout_sockets[1]);
    throw std::runtime_error("socketpair for stderr");
  }

  pid_t pid = fork();
  if (pid < 0) {
    close(stdout_sockets[0]);
    close(stdout_sockets[1]);
    close(stderr_sockets[0]);
    close(stderr_sockets[1]);
    throw std::runtime_error("fork");
  }

  if (pid == 0) {
    close(stdout_sockets[0]);
    close(stderr_sockets[0]);
    setup_child(stdout_sockets[1], stderr_sockets[1], pathName, scriptName, env);
    exit(EXIT_FAILURE);
  }
  
  close(stdout_sockets[1]);
  close(stderr_sockets[1]);
  
  std::cout << "stdout socket: " << stdout_sockets[0] << ", stderr socket: " << stderr_sockets[0] << std::endl;
  proc = new CGIProcess(stdout_sockets[0], stderr_sockets[0], pid);
  return proc;
}

void CGIHandler::setup_child(int stdout_sock, int stderr_sock, std::string &pathName, std::string &scriptName, char **env) const
{
  if (signal(SIGINT, SIG_IGN) == SIG_ERR)
    return (perror("signal"), exit(EXIT_FAILURE));
    
  if (dup2(stdout_sock, STDIN_FILENO) == -1 || dup2(stdout_sock, STDOUT_FILENO) == -1) {
      std::cerr << "dup2 failed for stdout: " << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
  }
  
  // gedirect stderr to stderr socket
  if (dup2(stderr_sock, STDERR_FILENO) == -1) {
      std::cerr << "dup2 failed for stderr: " << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
  }

  close(stdout_sock);
  close(stderr_sock);
  
  std::string script_path(scriptName);
  size_t last_slash = scriptName.find_last_of('/');
  std::string script_dir = (last_slash != std::string::npos) ? scriptName.substr(0, last_slash) : ".";
  scriptName = (last_slash != std::string::npos) ? scriptName.substr(last_slash + 1) : scriptName; 
  
  if (chdir(script_dir.c_str()) == -1) {
      std::cerr << "chdir failed: " << strerror(errno) << std::endl;
      exit(EXIT_FAILURE);
  }
  
  char *const args[3] = {const_cast<char *const>(pathName.c_str()),
                        const_cast<char *const>(scriptName.c_str()), NULL};

  execve(args[0], args, env);
  std::cerr << "execve failed: " << strerror(errno) << std::endl;
  exit(EXIT_FAILURE);
}

ssize_t CGIProcess::write(HTTPBody &body)
{
  char buff[READ_BUFFER_SIZE];

  size_t leftoverBytes = m_SocketBuffer.read(buff, READ_BUFFER_SIZE);
  size_t rbytes = body.read(buff + leftoverBytes, READ_BUFFER_SIZE - leftoverBytes);

  size_t buffSize = leftoverBytes + rbytes;
  ssize_t sent = send(this->cgi_stdout_sock, buff, buffSize, 0);
  if (sent <= 0) {
    this->cleanup(true);
    return sent;
  }
  if (static_cast<size_t>(sent) < buffSize)
    m_SocketBuffer.write(buff + sent, buffSize - sent);

  return sent;
}

ssize_t CGIProcess::read(char *buff, size_t size)
{
  ssize_t received = recv(this->cgi_stdout_sock, buff, size, 0);

  if (received < 0) { // -1
    std::cout << "++++++++++++++++++++++++++++" << std::endl;
    std::cout << "received == < 0" << std::endl;
    // this->cleanup(true);
    return received;
  } else if (received == 0) {
    std::cout << "----------------------------" << std::endl;
    std::cout << "received == 0" << std::endl;
    // this->cleanup(false);
  }
  std::cout << "RECEIVED: " << received << std::endl;
  return received;
}

ssize_t CGIProcess::readStderr(char *buff, size_t size)
{
  ssize_t received = recv(this->cgi_stderr_sock, buff, size, 0);
  
  if (received < 0) {
    std::cout << "stderr read error" << std::endl;
    // this->cleanup(true);
    return received;
  } else if (received == 0) {
    std::cout << "stderr closed" << std::endl;
    // this->cleanup(false);
  }
  std::cout << "STDERR RECEIVED: " << received << std::endl;
  return received;
}

void CGIProcess::cleanup(bool error) {
  if (error) {
    kill(pid, SIGKILL);
    std::cerr << "[CGI] Forcefully terminated PID " << pid << std::endl;
  }

  int status;
  if (waitpid(pid, &status, 0) == 0) {
    // Process still running, wait normally
  }

  if (cgi_stdout_sock != -1) {
    close(cgi_stdout_sock);
    cgi_stdout_sock = -1;
  }
  
  if (cgi_stderr_sock != -1) {
    close(cgi_stderr_sock);
    cgi_stderr_sock = -1;
  }

  if (WIFEXITED(status)) {
    std::cout << "[CGI] Process " << pid << " exited with status "
              << WEXITSTATUS(status) << std::endl;
  }
}