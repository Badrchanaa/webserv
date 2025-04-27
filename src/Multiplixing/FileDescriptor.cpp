#include "../../includes/FileDescriptor.hpp"
#include "../../includes/WebServer.hpp"


int FileDescriptor::release() {
  int old_fd = fd;
  fd = -1;
  return old_fd;
}

FileDescriptor::FileDescriptor(int f) : fd(f) {

}
FileDescriptor::~FileDescriptor() {
  this->reset();
}

void FileDescriptor::reset(int newfd) {
  if (fd != -1)
    close(fd);
  fd = newfd;
}

