#include "HTTPBody.hpp"
#include <iostream>
#include <string>

HTTPBody::HTTPBody(void)
{
}

HTTPBody::HTTPBody(char *buffer, size_t len)
{
	this->append(buffer, len);
}

HTTPBody::_appendToFile(const char *buff, size_t len)
{

}

HTTPBody::_appendToBuffer;

HTTPBody::append(char *buffer, size_t len)
{
	if (!m_IsFile)
	{
		if (len > MAX_BODY_MEMORY || m_Size + len > MAX_BODY_MEMORY)
		{
			this->_switchToFile();
			_appendToFile(const char *buff, size_t len);
		}
	}
	else
	{
	}
}

HTTPBody::~HTTPBody(void)
{
	
}