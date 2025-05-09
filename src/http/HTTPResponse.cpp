#include "HTTPResponse.hpp"
#include <iostream>
#include <string>
#include <sys/stat.h>

HTTPResponse::pollState	HTTPResponse::getPollState() const
{
	return m_PollState;
}

HTTPResponse::HTTPResponse(void): m_State(INIT), m_CursorPos(0), m_HasCgi(false)
{
	
}

bool	HTTPResponse::_isCgiPath(const std::string path, const ConfigServer *configServer)
{
	(void)path;
	(void)configServer;
	return false;
}
		
void	HTTPResponse::_initCgi(const std::string path, const CGIHandler &cgihandler, const ConfigServer *configServer)
{
	(void)path;
	(void)configServer;
	(void)cgihandler;
}

void	HTTPResponse::_initBadRequest()
{
	m_StatusCode = BAD_REQUEST;
	appendBody(std::string("<html><body><h1>400 Bad Request</h1></body></html>"));
	// TODO:: set headers
	addHeader("content-type", "text/html");
	addHeader("content-length", m_Body.getSize());
}

void	HTTPResponse::init(HTTPRequest const &request, CGIHandler const &cgihandler, ConfigServer const *configServer, int fd)
{
	std::string requestPath = request.getPath();
	struct stat fileStat;

	m_ClientFd = fd;
	this->m_Request = &request;
	// if (request.isError())
	// 	return _initBadRequest();
	requestPath = configServer->location.root + configServer->location.uri + requestPath;
	if (_isCgiPath(requestPath, configServer))
		return _initCgi(requestPath, cgihandler, configServer);
	if (access(requestPath.c_str(), R_OK) != 0)
		m_StatusCode = NOT_FOUND;
	else if (stat(requestPath.c_str(), &fileStat) == -1)
		m_StatusCode = SERVER_ERROR;
	else
		m_StatusCode = OK;
	m_State = PROCESS_BODY;
}


void		HTTPResponse::reset()
{
	return;
}

bool		HTTPResponse::isDone() const
{
	return m_State == DONE;
}

bool		HTTPResponse::isKeepAlive() const
{
	return false;
}


bool		HTTPResponse::hasCgi() const
{
	return m_HasCgi;
}

HTTPResponse::statusCode	HTTPResponse::getStatus()
{
	return OK;
}

HTTPResponse::responseState HTTPResponse::getState() const
{
	return m_State;
}

const std::string	HTTPResponse::_statusToString() const
{
	switch (m_StatusCode)
	{
		case 200:
			return std::string("OK");
		case 201:
			return std::string("Created");
		case 400:
			return std::string("Bad Request");
		case 403:
			return std::string("Forbidden");
		case 404:
			return std::string("Not Found");
		case 500:
			return std::string("Internal Server Error");
		default:	
			return std::string("UNKNOWN STATUS");
	}
}

void	HTTPResponse::_sendHeaders()
{
	int			writtenBytes;
	size_t		len;
	const char*	buff;
	std::string	headersString;

	headersString = m_HeadersStream.str();
	buff = headersString.c_str();
	len = headersString.length() - m_CursorPos;
	writtenBytes = send(m_ClientFd, buff + m_CursorPos, len, 0);
	if (writtenBytes < 0) // ERR ??
	{
		m_State = DONE;
		return;
	}
	if (m_CursorPos + writtenBytes == headersString.length())
	{
		m_State = SEND_BODY;
		m_CursorPos = 0;
	}
	else
		m_CursorPos += writtenBytes;
}

void	HTTPResponse::_processHeaders()
{
	int					statusCode;

	statusCode = static_cast<int>(m_StatusCode);
	m_HeadersStream << "HTTP/1.1 " << statusCode << " " << _statusToString() << CRLF;
	for (HTTPRequest::HeaderMap::iterator it = m_Headers.begin(); it != m_Headers.end(); it++)
	{
		m_HeadersStream << it->first << ": ";
		m_HeadersStream << it->second << CRLF;
	}
	m_HeadersStream << CRLF;
}

void	HTTPResponse::_processBody()
{
	m_CursorPos = 0;
	std::stringstream responseStream;

	std::cout << "PROCESS BODY START " << std::endl;
	responseStream << "<html><body>";
	responseStream << "<style>td{padding: 10;border: 2px solid black;background: #ababed;font-size:16;font-style:italic;}</style>";
	if (m_Request->isError())
		responseStream << "<h4>PARSE ERROR</h4>";
	else
		responseStream << "<h4>PARSE SUCCESS</h4>";
	responseStream << "<h4>method: " << m_Request->getMethodStr() << "<h4>";
	responseStream << "request path: '" << m_Request->getPath() << "'" << std::endl;
	responseStream << "<table>";
	responseStream << "<h1>REQUEST HEADERS</h1>";
	HTTPRequest::HeaderMap headers = m_Request->getHeaders();
	for (HTTPRequest::HeaderMap::iterator it = headers.begin(); it != headers.end(); it++)
	{
		responseStream << "<tr>";
		responseStream << "<td>" << it->first << "</td>";
		responseStream << "<td>" << it->second << "</td>";
		responseStream << "</tr>";
	}
	responseStream << "</table>";
	responseStream << "</body></html>";
	responseStream << "<h1>REQUEST BODY:</h1>";
	appendBody(responseStream.str());

	appendBody(m_Request->getBody().getBuffer());
	std::cout << "BODY PROCESSING END (size:" << m_Body.getSize() << ")" << std::endl;

	addHeader("content-type", "text/html");
	m_State = PROCESS_HEADERS;
}

void	HTTPResponse::_sendBody()
{
	const std::vector<char>	&bodyBuffer = m_Body.getBuffer();
	const char * buff = static_cast<const char *>(&bodyBuffer[m_CursorPos]);
	std::size_t	size = bodyBuffer.size();
	// write(1, buff, size);

	ssize_t	bytesWritten = send(m_ClientFd, buff, size - m_CursorPos, 0);
	if (bytesWritten < 0)
	{
			m_State = DONE;
			return ;
	}
	std::cout << "body size: " << m_Body.getSize() << std::endl;
	std::cout << "written: " << bytesWritten << " cursorPos: " << m_CursorPos << std::endl;
	if (bytesWritten + m_CursorPos < size)
	{
		m_CursorPos = bytesWritten;
		std::cout << "written: " << bytesWritten << " cursorPos: " << m_CursorPos << std::endl;
	}
	else
		m_State = DONE;
}

bool HTTPResponse::resume(bool isCgiReady, bool isClientReady)
{
	if (isCgiReady)
		std::cout << "CGI READY" << std::endl;
	if (isClientReady)
		std::cout << "CLIENT READY" << std::endl;

	// RESPONSE PROCESSING
	if (m_State == PROCESS_BODY)
		_processBody();
	if (m_State == PROCESS_HEADERS)
		_processHeaders();

	// SENDING RESPONSE
	if (m_State == SEND_HEADERS)
		_sendHeaders();
	else if (m_State == SEND_BODY)
		_sendBody();
	
	// CHECKING STATE
	if (m_State == DONE)
		std::cout << "RESPONSE COMPLETE" << std::endl;	
	else
		std::cout << "RESPONSE ONGOING" << std::endl;	
	return m_State == DONE;
}


// HTTPResponse::HTTPResponse(const HTTPResponse &other)
// {
	
// }

// HTTPResponse& HTTPResponse::operator=(const HTTPResponse &other)
// {
	
// }

HTTPResponse::~HTTPResponse(void)
{
	
}