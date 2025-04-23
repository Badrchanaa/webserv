#include "../../includes/FileDescriptor.hpp"
#include "../../includes/WebServer.hpp"


  explicit FileDescriptor::FileDescriptor(int f = -1) : fd(f) {}
  ~FileDescriptor::FileDescriptor() {this->reset(); }

  void FileDescriptor::reset(int new_fd = -1) {
    if (fd != -1)
      close(fd);
    fd = new_fd;
  }

  int FileDescriptor::release() {
    int old_fd = fd;
    fd = -1;
    return old_fd;
  }

