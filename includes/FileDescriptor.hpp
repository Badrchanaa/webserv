#ifndef __FileDescriptor__
#define __FileDescriptor__

class FileDescriptor {
public:
  int fd;

  explicit FileDescriptor(int); 
  ~FileDescriptor();

  void reset(int);

  int release();

  operator int() const { return fd; }

private:
  FileDescriptor(const FileDescriptor &);
  FileDescriptor &operator=(const FileDescriptor &);
};

#endif
