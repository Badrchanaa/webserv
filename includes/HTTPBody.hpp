#ifndef __HTTPBody_HPP__
#define __HTTPBody_HPP__

#include <fstream>
// #include <stdint.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include "RingBuffer.hpp"

#define KB (1024U)
#define MB (KB * KB)

#define MAX_BODY_MEMORY (5 * KB)

class HTTPBody
{
  public:
    HTTPBody();
    HTTPBody(char *buffer, size_t len);
    ~HTTPBody();

    bool append(const char *buffer, size_t len);
    // unsigned int				write(int fd);
    // const char *getBuffer() const;
    void flush();
    size_t getSize() const;
    int	read(char *buffer, size_t len);
    void   clear()
    {
      // TODO: Clear buffer
    }

  private:
    HTTPBody(HTTPBody &body);
    bool _switchToFile();
    bool _writeToBuffer(const char *buffer, size_t len);
    bool _writeToFile(const char *buffer, size_t len);

    std::string m_Filename;
    std::fstream m_File;
    // std::vector<char> m_VectorBuffer;
    RingBuffer<char>  m_RingBuffer;
    size_t m_Size;
    bool m_IsFile;
};

#endif
