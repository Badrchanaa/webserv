#include "HTTPRequest.hpp"
#include <iostream>
#include <string>

HTTPParseState &HTTPRequest::getParseState()
{
	return m_ParseState;
}

HTTPRequest::HTTPRequest(void)
{
	m_ParseState.setPrevChar('\n');
}

void	HTTPRequest::setMethod(char *method_cstr)
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

void	HTTPRequest::appendToPath(char *buff, size_t start, size_t len)
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