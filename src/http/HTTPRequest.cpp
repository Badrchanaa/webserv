#include "HTTPRequest.hpp"
#include "HTTPParser.hpp"
#include <iostream>
#include <string>
#include <sstream>

HTTPRequest::HTTPRequest(std::vector<ConfigServer> &servers): HTTPMessage(),
	multipartForm(NULL), m_isMultipartForm(false),  m_Error(ERR_NONE), 
	 m_TransferEncoding(DEFAULT), m_ConfigServers(servers), m_ConfigServer(NULL)
{
}

const ConfigServer*	HTTPRequest::getServer() const
{
	return m_ConfigServer;
}

bool	HTTPRequest::isTransferChunked() const
{
	return m_TransferEncoding == CHUNKED;
}

bool	HTTPRequest::isError() const
{
	HTTPParseState::state_t state = m_ParseState.getState();
	return state == HTTPParseState::PARSE_ERROR;
}

void	HTTPRequest::onBodyDone()
{
	m_Body.seal();
	m_ParseState.setState(HTTPParseState::PARSE_DONE);
}

void	HTTPRequest::_checkMultipart()
{
	if (m_ConfigServer->getLocation(m_Path)->isCgiPath(m_Path))
		return ;
	if (!hasHeader("content-type"))
		return ;
	std::string contentType = getHeader("content-type");
	std::string::size_type pos = contentType.find(';');
	if (contentType.substr(0, pos) != "multipart/form-data")
		return ;
	HTTPRequest::header_map_t mediaTypes = parseHeaderDirectives(contentType, pos);
	HTTPRequest::header_map_t::const_iterator it;
	it = mediaTypes.find("boundary");
	if (it == mediaTypes.end() || it->second.empty())
	{
		std::cout << "BOUNDARY NOT FOUND" << std::endl;
		return m_ParseState.setError();
	}
	m_isMultipartForm = true;
	if (multipartForm)
	{
		std::cout << "multipart check called again" << std::endl;
		delete multipartForm;
	}
	multipartForm = new HTTPMultipartForm(mediaTypes);
}

void	HTTPRequest::onHeadersParsed()
{
	header_map_t::const_iterator	it;

	size_t pos = m_Uri.find('?');
	if (pos != std::string::npos)
	{
		m_Path = m_Uri.substr(0, pos);
		m_Query.assign(&m_Uri[pos + 1]);
		std::cout << "REQUEST PATH: " << m_Path << std::endl;
		std::cout << "REQUEST QUERY: " << m_Query << std::endl;
	}
	else
		m_Path = m_Uri;
	for(it = m_Headers.begin(); it != m_Headers.end(); it++)
	{
		// std::cout << "headers[" << it->first << "] = " << it->second << std::endl;
	}
	if (!this->_validateHeaders())
		m_ParseState.setError();
	if (m_Error == ERR_INVALID_CONTENT_LENGTH)
		std::cout << "INVALID CONTENT LENGTH" << std::endl;
	if (m_Error == ERR_INVALID_HOST)
		std::cout << "INVALID HOST" << std::endl;
	if (m_Method == GET || m_Method == HEAD || m_ContentLength == 0)
		m_ParseState.setState(HTTPParseState::PARSE_DONE);
	else
		m_ParseState.setState(HTTPParseState::PARSE_BODY);
	m_ConfigServer = &(Config::getServerByName(m_ConfigServers, m_Headers["host"]));
	_checkMultipart();
}

bool	HTTPRequest::isMultipartForm() const
{
	return m_isMultipartForm;
}

/*
	Process and validate request headers (Host, Content-length, etc..)
	returns if headers are valid.
*/
bool	HTTPRequest::_checkTransferChunked()
{
	header_map_t::const_iterator	it;
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
	header_map_t::const_iterator	it;
	std::istringstream			iss;

	it = m_Headers.find("host");
	if (it == m_Headers.end())
	{
		m_Error = ERR_INVALID_HOST;
		return false;
	}
	isChunked = _checkTransferChunked();
	m_Host = it->second;
	it = m_Headers.find("content-length");
	if (it != m_Headers.end() && isChunked) // has both content-length and chunked, should reject
	{
		m_Error = ERR_INVALID_CONTENT_LENGTH;
		return false;
	}
	if (m_Method != GET && m_Method != HEAD && it == m_Headers.end() && !isChunked)
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

void	HTTPRequest::setMethod(const char *method_cstr)
{
	std::string method(method_cstr);
	/*
		TODO: check if method in allowed methods
	 		  otherwise set parseState to PARSE_ERROR ? and status code to 501 not implemented
	*/
	if (method == "GET")	
		m_Method = GET;
	else if (method == "POST")
		m_Method = POST;
	else if (method == "PUT")
		m_Method = PUT;
	else if (method == "HEAD")
		m_Method = HEAD;
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
		case HEAD:
			return "HEAD";
		case DELETE:
			return "DELETE";
		default:
			return "UNKNOWN_METHOD";
	}
}

bool	HTTPRequest::validUri()
{
	return m_Uri[0] == '/';
}

void	HTTPRequest::reset()
{
	m_Path.clear();
	m_Headers.clear();
}

void	HTTPRequest::appendUri(const char *buff, size_t start, size_t len)
{
	// std::cout << "APPENDING TO PATH: start: " << start << " len: " << len << std::endl;
	for (size_t i = start; i < len; i++)
	{
		m_Uri += buff[i];
	}
	// m_Path.append(buff + start, 0, len - start);
}

const std::string		&HTTPRequest::getPath() const
{
	return m_Path;
}

const std::string		&HTTPRequest::getUri() const
{
	return m_Uri;
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
	// std::cout << "request destructor called" << std::endl;
	delete multipartForm;
    this->forceCleanup();
}
