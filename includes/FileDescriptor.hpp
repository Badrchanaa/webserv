#ifndef __FileDescriptor__
#define __FileDescriptor__

#include <iostream>

class FileDescriptor {
public:
  int fd;

  explicit FileDescriptor(int fd = -1);
  ~FileDescriptor(){
    this->reset();
  }

  void reset(int newfd = -1);
  int release();

  operator int() const { return fd; }

private:
  // Prevent copying
  FileDescriptor(const FileDescriptor&);
  FileDescriptor& operator=(const FileDescriptor&);
};

#endif
