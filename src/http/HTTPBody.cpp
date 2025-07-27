#include "HTTPBody.hpp"
#include <iostream>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <cstring>

extern int errno;

HTTPBody::HTTPBody(void): m_RingBuffer(MAX_BODY_MEMORY), m_Size(0), m_IsFile(false)
{
	std::cout << "new body" << std::endl;
}

HTTPBody::HTTPBody(char *buffer, size_t len): m_RingBuffer(MAX_BODY_MEMORY), m_Size(0), m_IsFile(false)
{
	std::cout << "new body cp" << std::endl;
	this->append(buffer, len);
}

size_t	HTTPBody::getSize() const
{
	return m_Size;
}
// const char*	HTTPBody::getBuffer() const
// {
// 	const char*	buff = &m_VectorBuffer[m_Offset];
// 	return buff;
// }

bool	HTTPBody::_writeToFile(const char *buffer, size_t len)
{
	m_File.write(buffer, len);
	m_Size += len;
	return static_cast<bool>(m_File);
}

bool HTTPBody::_switchToFile()
{
	// m_File.open("body.http", std::ios::in | std::ios::binary);
	std::string filename = std::tmpnam(NULL);
	std::cout << "switched to file: " << filename << std::endl;
	m_File.open(filename.c_str(), std::ios::out | std::ios::in | std::ios::binary);
	if (!m_File.is_open())
	{
		std::cout << "failed to open file" << std::endl;	
		std::cout << "error: " << strerror(errno) << std::endl;
		return false;
	}
	m_IsFile = true;
	// m_File.write(&m_VectorBuffer[0], m_Size);
	return static_cast<bool>(m_File);
}

void	HTTPBody::flush()
{
	if (m_IsFile)
		m_File.flush();
}

int	HTTPBody::read(char *buffer, size_t len)
{
	size_t	rbytes;

	rbytes = m_RingBuffer.read(buffer, len);
	if (rbytes == len)
		return len;
	m_File.read(buffer + rbytes, len - rbytes);
	return len;
}

bool	HTTPBody::_writeToBuffer(const char *buffer, size_t len)
{
	m_RingBuffer.write(buffer, len);
	m_Size += len;
	return true;
}

bool	HTTPBody::append(const char *buffer, size_t len)
{
	if (len == 0)
		return true;
	if (m_IsFile)
		return _writeToFile(buffer, len);
	if (m_Size + len > MAX_BODY_MEMORY)
	{
		if (!this->_switchToFile())
			return false;
		return _writeToFile(buffer, len);
	}
	_writeToBuffer(buffer, len);
	return true;
}

HTTPBody::~HTTPBody(void)
{
	if (m_IsFile)
		m_File.close();
}
