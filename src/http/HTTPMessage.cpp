#include "HTTPMessage.hpp"
#include <iostream>
#include <string>
#include <sstream>

HTTPMessage::HTTPMessage(void)
{
	
}

const HTTPBody&	HTTPMessage::getBody() const
{
	return m_Body;
}

bool	HTTPMessage::hasHeader(const char *key) const
{
	return m_Headers.count(key) != 0;
}

size_t		HTTPMessage::getContentLength() const
{
	return m_ContentLength;
}

const HTTPMessage::HeaderMap&	HTTPMessage::getHeaders() const
{
	return this->m_Headers;
}

void	HTTPMessage::addHeader(std::string key, size_t value)
{
	std::stringstream ss;

	ss << value;
	addHeader(key, ss.str());
}

void	HTTPMessage::addHeader(std::string key, std::string value)
{
	size_t	start = value.find_first_not_of(" \t");
	size_t	end = value.find_last_not_of(" \t");
	if (start == std::string::npos || start == end)
	{
		m_Headers[key];
		return;
	}
	m_Headers[key] = value.substr(start, end + 1);
}

std::string			HTTPMessage::getHeader(const char *key) const
{
	std::string s(key);
	return getHeader(s);
}

void	HTTPMessage::appendBody(const char *buff, size_t start, size_t end)
{
	m_Body.append(buff + start, end);	
}

void	HTTPMessage::appendBody(const std::string str)
{
	size_t		size;
	const char	*strBuff;

	size = str.length();
	strBuff = str.c_str();
	m_Body.append(strBuff , size);
}

void	HTTPMessage::appendBody(const std::vector<char>& vec)
{
	m_Body.append(&vec[0], vec.size());
}

std::string	HTTPMessage::getHeader(std::string &key) const
{
	HeaderMap::const_iterator it = m_Headers.find(key);
	if (it == m_Headers.end())
		return std::string();
	return it->second;	
}

// HTTPMessage::HTTPMessage(const HTTPMessage &other)
// {
	
// }

// HTTPMessage& HTTPMessage::operator=(const HTTPMessage &other)
// {
	
// }

HTTPMessage::~HTTPMessage(void)
{
	
}