#include "HTTPMessage.hpp"
#include <iostream>
#include <string>

HTTPMessage::HTTPMessage(void) 
{
}

HTTPParseState &HTTPMessage::getParseState()
{
	return m_ParseState;
}

bool	HTTPMessage::isParseComplete() const
{
	return m_ParseState.isComplete();
}

HTTPBody&	HTTPMessage::getBody()
{
	return m_Body;
}


size_t		HTTPMessage::getContentLength() const
{
	return m_ContentLength;
}


bool	HTTPMessage::appendBody(const char *buff, size_t len)
{
	return m_Body.append(buff, len);
}

bool	HTTPMessage::appendBody(const std::string str)
{
	size_t		size;
	const char	*strBuff;

	size = str.length();
	strBuff = str.c_str();
	return m_Body.append(strBuff , size);
}

bool	HTTPMessage::appendBody(const std::vector<char>& vec)
{
	return m_Body.append(&vec[0], vec.size());
}

HTTPMessage::~HTTPMessage(void)
{
	
}