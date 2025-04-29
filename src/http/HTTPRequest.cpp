#include "HTTPRequest.hpp"
#include <iostream>
#include <string>
#include <sstream>

HTTPParseState &HTTPRequest::getParseState()
{
	return m_ParseState;
}

HTTPRequest::HTTPRequest(void): m_Error(ERR_NONE), m_ContentLength(0), m_TransferEncoding(DEFAULT), m_MultipartForm(NULL)
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

bool	HTTPRequest::isError() const
{
	HTTPParseState::requestState state = m_ParseState.getState();
	return state == HTTPParseState::REQ_ERROR;
}

const HTTPBody&	HTTPRequest::getBody() const
{
	return m_Body;
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
		m_ParseState.setError();
	if (m_Error == ERR_INVALID_CONTENT_LENGTH)
		std::cout << "INVALID CONTENT LENGTH" << std::endl;
	if (m_Error == ERR_INVALID_HOST)
		std::cout << "INVALID HOST" << std::endl;
	if (m_Method == GET)
		m_ParseState.setState(HTTPParseState::REQ_DONE);
}

bool	HTTPRequest::isMultipartForm() const
{
	return false;
}

size_t		HTTPRequest::getContentLength() const
{
	return m_ContentLength;
}

// std::string HTTPRequest::getBodyStr() const
// {
	
// }

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
	{
		m_Error = ERR_INVALID_CONTENT_LENGTH;
		return false;
	}
	if (it == m_Headers.end())
		return true;
	iss.str(it->second);
	if (!(iss >> m_ContentLength) || !iss.eof())
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
	if (method == "GET")	
		m_Method = GET;
	else if (method == "POST")
		m_Method = POST;
	else if (method == "PUT")
		m_Method = PUT;
	else if (method == "DELETE")
		m_Method = DELETE;
	else // unsupported method
	{
		m_Error = ERR_INVALID_METHOD;
		m_ParseState.setError();
	}
}

const char*	HTTPRequest::getMethodStr() const
{
	switch(m_Method)
	{
		case GET:
			return "GET";
		case POST:
			return "POST";
		case PUT:
			return "PUT";
		case DELETE:
			return "DELETE";
		default:
			return "UNKNOWN_METHOD";
	}
}

bool	HTTPRequest::validPath()
{
	// TODO: validate path correctly
	if (m_Path[0] != '/')
		return false;
	return true;
}

void	HTTPRequest::reset()
{
	m_Path.clear();
	m_Headers.clear();
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