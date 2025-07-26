#ifndef __HTTPBody_HPP__
#define __HTTPBody_HPP__

#include <fstream>
// #include <stdint.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>

#define MB (1024U * 1024U)

#define MAX_BODY_MEMORY (2 * MB)

class HTTPBody
{
  public:
    HTTPBody();
    HTTPBody(HTTPBody &body)
    {
	    std::cout << "new body cp2" << std::endl;
      // m_Filename = body.m_Filename;
      // m_IsFile = body.m_IsFile;
      // m_Size = body.m_Size;
      // m_VectorBuffer = body.m_VectorBuffer;
      // if (m_IsFile)
      // {
	    //   m_File.open(m_Filename, std::ios::in | std::ios::binary);
	    //   if (!m_File.is_open())
      //     std::cerr << "could not open file" << std::endl;
      // }
      // return;
      return;
    }
    HTTPBody(char *buffer, size_t len);
    ~HTTPBody();

    bool append(const char *buffer, size_t len);
    // unsigned int				write(int fd);
    const char *getBuffer() const;
    void flush();
    size_t getSize() const;
    void setOffset(size_t offset);
    size_t getOffset();
    void   clear()
    {
      m_VectorBuffer.clear();
    }

  private:
    bool _switchToFile();
    bool _writeToBuffer(const char *buffer, size_t len);
    bool _writeToFile(const char *buffer, size_t len);

    std::string m_Filename;
    std::ofstream m_File;
    std::vector<char> m_VectorBuffer;
    size_t m_Size;
    bool m_IsFile;
    size_t m_Offset;
};

#endif
