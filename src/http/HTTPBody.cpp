#include "HTTPBody.hpp"
#include <iostream>
#include <string>

HTTPBody::HTTPBody(void)
{
	m_VectorBuffer.reserve(1024);
}

HTTPBody::HTTPBody(char *buffer, size_t len)
{
	this->append(buffer, len);
}

bool	HTTPBody::_writeToFile(const char *buffer, size_t len)
{
	m_File.write(buffer, len);
	m_Size += len;
	return static_cast<bool>(m_File);
}

bool HTTPBody::_switchToFile()
{
	m_File.open(std::tmpnam(NULL), std::ios::in | std::ios::binary);
	if (!m_File.is_open())
		return false;
	m_File.write(&m_VectorBuffer[0], m_Size);
	return static_cast<bool>(m_File);
}

void	HTTPBody::flush()
{
	if (m_IsFile)
		m_File.flush();
}

bool	HTTPBody::_writeToBuffer(const char *buffer, size_t len)
{
	m_VectorBuffer.insert(m_VectorBuffer.end(), buffer, buffer + len);
	m_Size += len;
	return true;
}

bool	HTTPBody::append(const char *buffer, size_t len)
{
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