#include "HTTPRequest.hpp"
#include <iostream>
#include <string>
#include <sstream>

HTTPParseState &HTTPRequest::getParseState()
{
	return m_ParseState;
}

HTTPRequest::HTTPRequest(void): m_ContentLength(0), m_TransferEncoding(DEFAULT), m_MultipartForm(NULL)
{
	m_ParseState.setPrevChar('\n');
}

bool	HTTPRequest::isTransferChunked() const
{
	return m_TransferEncoding == CHUNKED;
}

bool	HTTPRequest::isComplete() const
{
	HTTPParseState::requestState state = m_ParseState.getState();
	return state == HTTPParseState::REQ_DONE || state == HTTPParseState::REQ_ERROR;
}

bool	HTTPRequest::appendBody(const char *buff, size_t len)
{
	return m_Body.append(buff, len);
}

bool	HTTPRequest::hasHeader(const char *key) const
{
	return m_Headers.count(key) != 0;
}

void	HTTPRequest::processHeaders()
{
	HeaderMap::const_iterator	it;

	if (!this->_validateHeaders())
		return;
}

bool	HTTPRequest::isMultipartForm() const
{
	return false;
}

/*
	Process and validate request headers (Host, Content-length, etc..)
	returns if headers are valid.
*/
bool	HTTPRequest::_validateHeaders()
{
	HeaderMap::const_iterator	it;
	std::istringstream			iss;

	it = m_Headers.find("host");
	if (it == m_Headers.end())
	{
		m_Error = ERR_INVALID_HOST;
		return false;
	}
	m_Host = it->second;
	it = m_Headers.find("content-length");
	if (it == m_Headers.end() && !hasHeader("transfer-encoding"))
		return ERR_INVALID_CONTENT_LENGTH;
	iss.str(it->second);
	if (!iss >> m_ContentLength || !iss.eof())
	{
		m_Error = ERR_INVALID_CONTENT_LENGTH;
		return false;
	}
	return m_Error == ERR_NONE;
}

const HTTPRequest::HeaderMap&	HTTPRequest::getHeaders() const
{
	return this->m_Headers;
}

void	HTTPRequest::addHeader(std::string key, std::string value)
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

std::string			HTTPRequest::getHeader(const char *key) const
{
	std::string s(key);
	return getHeader(s);
}

std::string	HTTPRequest::getHeader(std::string &key) const
{
	HeaderMap::const_iterator it = m_Headers.find(key);
	if (it == m_Headers.end())
		return std::string();
	return it->second;	
}

void	HTTPRequest::setMethod(const char *method_cstr)
{
	std::string method(method_cstr);
	/*
		TODO: check if method in allowed methods
	 		  otherwise set parseState to REQ_ERROR ? and status code to 501 not implemented
	*/
}

bool	HTTPRequest::validPath()
{
	// TODO: validate path correctly
	if (m_Path[0] != '/')
		return false;
	return true;
}

void	HTTPRequest::appendToPath(const char *buff, size_t start, size_t len)
{
	m_Path.append(buff, start, len - start);
}

HTTPRequest::HTTPRequest(const HTTPRequest &other)
{
	(void)other;
}

const std::string		&HTTPRequest::getPath() const
{
	return m_Path;
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest &other)
{
	(void)other;
	return *this;	
}

HTTPRequest::~HTTPRequest(void)
{
	
}