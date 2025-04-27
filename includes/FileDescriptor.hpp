#ifndef __FileDescriptor__
#define __FileDescriptor__

class FileDescriptor {
public:
  int fd;

  explicit FileDescriptor(int fd=-1); 
  ~FileDescriptor();

  void reset(int newfd=-1);

  int release();

  operator int() const { return fd; }

private:
  FileDescriptor(const FileDescriptor &);
  // FileDescriptor &operator=(const FileDescriptor &);
};

#endif
