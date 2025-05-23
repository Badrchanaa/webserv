#include "HTTPRequest.hpp"
#include <iostream>
#include <string>
#include <sstream>

HTTPParseState &HTTPRequest::getParseState()
{
	return m_ParseState;
}

HTTPRequest::HTTPRequest(std::vector<ConfigServer> &servers): HTTPMessage(),
	m_Error(ERR_NONE), m_TransferEncoding(DEFAULT),
	m_MultipartForm(NULL), m_ConfigServers(servers), m_ConfigServer(NULL)
{
	m_ParseState.setPrevChar('\n');
}

const ConfigServer*	HTTPRequest::getServer() const
{
	return m_ConfigServer;
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

void	HTTPRequest::processHeaders()
{
	HeaderMap::const_iterator	it;

	// TODO: do not ignore request parameters!!
	size_t pos = m_Path.find('?');
	if (pos != std::string::npos)
		m_Path.substr(0, pos);

	if (!this->_validateHeaders())
		m_ParseState.setError();
	if (m_Error == ERR_INVALID_CONTENT_LENGTH)
		std::cout << "INVALID CONTENT LENGTH" << std::endl;
	if (m_Error == ERR_INVALID_HOST)
		std::cout << "INVALID HOST" << std::endl;
	m_ConfigServer = &(Config::getServerByName(m_ConfigServers, m_Headers["host"]));
	if (m_Method == GET)
		m_ParseState.setState(HTTPParseState::REQ_DONE);
}

bool	HTTPRequest::isMultipartForm() const
{
	return false;
}

/*
	Process and validate request headers (Host, Content-length, etc..)
	returns if headers are valid.
*/

bool	HTTPRequest::_checkTransferChunked()
{
	HeaderMap::const_iterator	it;
	size_t						start;
	size_t						end;
	const std::string			chunkedStr = "chunked";

	it = m_Headers.find("transfer-encoding");
	if (it == m_Headers.end())
		return false;
	std::string transferEncoding = it->second;
	start = transferEncoding.find_first_not_of(" ");
	if (start == std::string::npos)
	{
		m_Error = ERR_UNIMPLEMENTED_TE;
		m_ParseState.setError();
		return false;
	}
	end = transferEncoding.find_last_not_of(" ");
	transferEncoding = transferEncoding.substr(start, end + 1);
	if (chunkedStr.size() != transferEncoding.size())
	{
		std::cout << "here" <<std::endl;
		m_Error = ERR_UNIMPLEMENTED_TE;
		m_ParseState.setError();
		return false;
	}
	for (size_t i = 0; i < chunkedStr.length(); i++)
	{
		if (std::tolower(chunkedStr[i]) != std::tolower(transferEncoding[i]))
		{
			m_Error = ERR_UNIMPLEMENTED_TE;
			m_ParseState.setError();
			return false;
		}
	}
	m_TransferEncoding = CHUNKED;
	return true;
}

bool	HTTPRequest::_validateHeaders()
{
	bool						isChunked;
	HeaderMap::const_iterator	it;
	std::istringstream			iss;

	it = m_Headers.find("host");
	if (it == m_Headers.end())
	{
		m_Error = ERR_INVALID_HOST;
		return false;
	}
	isChunked = _checkTransferChunked();
	if (isChunked)
		std::cout << "TRANSFER IS CHUNKED" << std::endl;
	m_Host = it->second;
	it = m_Headers.find("content-length");
	if (it != m_Headers.end() && isChunked) // has both content-length and chunked, should reject
	{
		m_Error = ERR_INVALID_CONTENT_LENGTH;
		return false;
	}
	if (m_Method != GET && it == m_Headers.end() && !isChunked)
	{
		std::cout << "CONTENT LENGTH NOT FOUND" << std::endl;
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
	std::cout << "APPENDING TO PATH: start: " << start << " len: " << len << std::endl;
	for (size_t i = start; i < len; i++)
	{
		m_Path += buff[i];
	}
	// m_Path.append(buff + start, 0, len - start);
}

// HTTPRequest::HTTPRequest(const HTTPRequest &other)
// {
// 	(void)other;
// }

const std::string		&HTTPRequest::getPath() const
{
	return m_Path;
}

httpMethod	HTTPRequest::getMethod() const
{
	return m_Method;
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest &other)
{
	(void)other;
	return *this;	
}

HTTPRequest::~HTTPRequest(void)
{
	
}