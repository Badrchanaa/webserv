
#include "HTTPHeaders.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

HTTPHeaders::HTTPHeaders(void)
{

}

HTTPHeaders::~HTTPHeaders(void)
{
}

bool	HTTPHeaders::hasHeader(const char *key) const
{
	return m_Headers.count(key) != 0;
}

bool	HTTPHeaders::removeHeader(const char *key)
{
	std::string skey(key);
	return m_Headers.erase(skey) > 0;
}

const HTTPHeaders::header_map_t&	HTTPHeaders::getHeaders() const
{
	return this->m_Headers;
}

void	HTTPHeaders::addHeader(std::string key, size_t value)
{
	std::stringstream ss;

	ss << value;
	addHeader(key, ss.str());
}

void	HTTPHeaders::addHeader(std::string key, std::string value)
{
	size_t	start = value.find_first_not_of(" \t");
	size_t	end = value.find_last_not_of(" \t");

	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::cout << "ADD HEADER: " << key << " = " << value << std::endl;
	if (start == std::string::npos)
	{
		std::cout << "value empty" << std::endl;
		m_Headers[key];
		return;
	}
	m_Headers[key] = value.substr(start, end + 1);
}

const std::string&	HTTPHeaders::getHeader(const char *key) const
{
	return getHeader(std::string(key));
}

const std::string&	HTTPHeaders::getHeader(const std::string &key) const
{
	header_map_t::const_iterator it = m_Headers.find(key);
	if (it == m_Headers.end())
		throw std::exception();
	return it->second;	
}