#include "HTTPResponse.hpp"
#include <iostream>
#include <string>
#include <sys/stat.h>

HTTPResponse::pollState	HTTPResponse::getPollState() const
{
	return m_PollState;
}

HTTPResponse::HTTPResponse(void): m_State(INIT), m_PollState(SOCKET_WRITE), m_CursorPos(0), 
				m_HasCgi(false), m_ConfigServer(NULL), m_Location(NULL)
{
	m_StatusCode = HTTPResponse::OK;
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

bool	_safePath(const std::string path)
{
	int		count;
	size_t	start;
	size_t	i;
	std::string subpath;

	start = 0;
	count = 0;
	for (i = 0; i < path.length(); i++)
	{
		if (path[i] == '/')
		{
			if (start == i)
			{
				start = i + 1;
				continue;
			}
			subpath = path.substr(start, i - start);
			if (subpath == "..")
				count--;
			else if (subpath != ".")
				count++;
			start = i + 1;
		}
	}
	if (start != i)
	{
		subpath = path.substr(start, i - start);
		if (subpath == "..")
			count--;
		else if (subpath != ".")
			count++;
	}
	std::cout << "count: " << count << std::endl;
	return count >= 0;
}

void	HTTPResponse::init(HTTPRequest const &request, CGIHandler const &cgihandler, ConfigServer const *configServer, int fd)
{
	std::string resourcePath;
	struct stat fileStat;

	m_ClientFd = fd;
	this->m_Request = &request;
	this->m_ConfigServer = configServer;
	
	m_State = PROCESS_BODY;
	if (request.isError())
	{
		m_StatusCode = HTTPResponse::BAD_REQUEST;
		return;
	}
	// if (request.isError())
	// 	return _initBadRequest();

	// check for path traversal
	if (!_safePath(request.getPath()))
	{
		std::cout << "NOT A SAFE PATH" << std::endl;
		m_StatusCode = NOT_FOUND;
		return;
	}
	m_Location = &configServer->getLocation(request.getPath());
	resourcePath = "./" + m_Location->root + m_Location->uri + request.getPath();
	std::cout << "RESOURCE PATH: " << resourcePath << std::endl;
	if (_isCgiPath(request.getPath(), configServer))
		return _initCgi(request.getPath(), cgihandler, configServer);
	
	if (request.getMethod() == GET)
	{
		if (access(resourcePath.c_str(), R_OK) != 0)
			m_StatusCode = NOT_FOUND;
		else if (stat(resourcePath.c_str(), &fileStat) == -1)
			m_StatusCode = SERVER_ERROR;
	}
	else if (request.getMethod() == POST)
		m_StatusCode = NOT_IMPLEMENTED;
	// ADD DEBUG INFO TO RESPONSE BODY
	// _debugBody();
	m_ResourcePath = resourcePath;
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
	std::cout << "headers size: " << headersString.length() << " cursorPos:" << m_CursorPos << std::endl;
	std::cout << "HEADERS: sent " << writtenBytes << " bytes to socket " << m_ClientFd << std::endl;
	if (writtenBytes < 0) // ERR ??
	{
		m_State = DONE;
		return;
	}
	if (m_CursorPos + writtenBytes == headersString.length())
	{
		std::cout << "SEND BODY//" << std::endl;
		m_State = SEND_BODY;
		m_CursorPos = 0;
	}
	else
		m_CursorPos += writtenBytes;
}

void	HTTPResponse::_processHeaders()
{
	int					statusCode;

	addHeader("content-length", m_Body.getSize());
	statusCode = static_cast<int>(m_StatusCode);
	m_HeadersStream << "HTTP/1.1 " << statusCode << " " << _statusToString() << CRLF;
	for (HTTPRequest::HeaderMap::iterator it = m_Headers.begin(); it != m_Headers.end(); it++)
	{
		m_HeadersStream << it->first << ": ";
		m_HeadersStream << it->second << CRLF;
	}
	m_HeadersStream << CRLF;
	m_State = SEND_HEADERS;
}

void	HTTPResponse::_debugBody()
{
	std::stringstream responseStream;

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

	appendBody(std::string("<h1>response body</h1>"));
	addHeader("content-type", "text/html");
}

const std::string  HTTPResponse::_getDefaultErrorFile() const
{
	std::map<HTTPResponse::statusCode,const char*> defaultPages;

	defaultPages[BAD_REQUEST] = "./html/error_400.html";
	defaultPages[NOT_FOUND] = "./html/error_404.html";
	defaultPages[FORBIDDEN] = "./html/error_403.html";
	defaultPages[SERVER_ERROR] = "./html/error_500.html";
	defaultPages[NOT_IMPLEMENTED] = "./html/error_501.html";

	return std::string(defaultPages[m_StatusCode]);
}

bool	HTTPResponse::_validFile(const std::string filename) const
{
	struct stat fileStat;

	if (access(filename.c_str(), R_OK) != 0 || stat(filename.c_str(), &fileStat) == -1)
		return false;
	return S_ISREG(fileStat.st_mode);
}

bool	HTTPResponse::_validDirectory(const std::string filename) const
{
	struct stat fileStat;

	if (access(filename.c_str(), R_OK | X_OK) != 0 || stat(filename.c_str(), &fileStat) == -1)
		return false;
	return S_ISDIR(fileStat.st_mode);
}

void	HTTPResponse::_readFileToBody(const std::string filename)
{
	std::ifstream	fileStream;
	char buff[4096];

	fileStream.open(filename.c_str());
	std::streamsize rbytes = 4096;
	while (rbytes >=4096)
	{
		fileStream.read(buff, 4096);
		rbytes = fileStream.gcount();
		std::cout << "read " << rbytes << " bytes from file " << std::endl;
		appendBody(buff, rbytes);
	}
}

void	HTTPResponse::_processErrorBody()
{
	std::cout << "PROCESS ERROR BODY" << std::endl;
	const std::map<int, std::string>	&errors = m_ConfigServer->errors;
	std::map<int, std::string>::const_iterator	it;
	std::string	filename;

	// Temporarily add quotes to look for key.
	it = errors.find(static_cast<int>(m_StatusCode));
	if (it == errors.end())
		filename = _getDefaultErrorFile();
	else
		filename = "./" + m_Location->root + "/" + it->second;
	std::cout << "FILENAME: " << filename << std::endl;
	if (!_validFile(filename))
	{
		std::string body;
		body = "<html><body><h1>Unexpected error";
		body += "</h1><p>(Default WebServ Error Page)</body></html>";
		appendBody(body);
		return;
	}
	else
		_readFileToBody(filename);
}

// PATH TRAVERSAL
void	HTTPResponse::_processDirectoryListing()
{
	DIR	*dir;
	struct dirent	*entry;
	std::string path;

	if (!m_Location->autoindex)
	{
		m_StatusCode = FORBIDDEN;
		return _processErrorBody();
	}
	dir = opendir(m_ResourcePath.c_str());
	if (!dir)
	{
		m_StatusCode = SERVER_ERROR;
		return _processErrorBody();
	}	
	std::cout << "DIRECTORY LISTING" << std::endl;
	std::stringstream body;
	path = m_Request->getPath();
	if (path[path.length() - 1] != '/')
		path += "/";

	body << "<html><title>" << m_Request->getPath() << " listing" << "</title><body>";
	body << "<h1>" << path << " listing</h1><ul>";
	entry = readdir(dir);
	while (entry)
	{
		body << "<li><a href=\"" << path << entry->d_name << "\">" << entry->d_name << "</a></li>";
		entry = readdir(dir);
	}
	body << "</ul></body></html>";
	appendBody(body.str());
	closedir(dir);
}

void	HTTPResponse::_processBody()
{
	m_CursorPos = 0;

	std::cout << "PROCESS BODY" << std::endl;
	if (m_StatusCode != OK)
		_processErrorBody();
	else if (m_Request->getMethod() == GET)
	{
		// TODO: error handling & large files (chunked response)
		if (_validFile(m_ResourcePath))
			_readFileToBody(m_ResourcePath);
		else if (_validDirectory(m_ResourcePath))
		{
			std::string indexPath = m_Location->getIndexPath(m_Request->getPath());
			if (indexPath == m_Request->getPath())
				_readFileToBody(indexPath);
			else
				_processDirectoryListing();
		}
		else
		{ 
			// if not a file nor a directory (or bad permissions).
			// return forbidden ??
			m_StatusCode = FORBIDDEN;
			_processErrorBody();
		}
	}

	m_State = PROCESS_HEADERS;
}

void	HTTPResponse::_sendBody()
{
	std::cout << "PROCESS BODY" << std::endl;
	const std::vector<char>	&bodyBuffer = m_Body.getBuffer();
	const char * buff = const_cast<const char *>(&bodyBuffer[m_CursorPos]);
	std::size_t	size = bodyBuffer.size();
	// write(1, buff, size);

	ssize_t	wBytes = send(m_ClientFd, buff, size - m_CursorPos, 0);
	if (wBytes < 0)
	{
			m_State = DONE;
			return ;
	}
	std::cout << "body size: " << m_Body.getSize() << std::endl;
	std::cout << "BODY: sent " << wBytes << " bytes to socket " << m_ClientFd << std::endl;
	if (wBytes + m_CursorPos < size)
	{
		m_CursorPos = wBytes;
		std::cout << "written: " << wBytes << " cursorPos: " << m_CursorPos << std::endl;
	}
	else
		m_State = DONE;
}

bool HTTPResponse::resume(bool isCgiReady, bool isClientReady)
{
	std::cout << "ENTER RESUME" << std::endl;
	if (isCgiReady)
		std::cout << "CGI READY" << std::endl;
	if (isClientReady)
		std::cout << "CLIENT READY" << std::endl;

	// RESPONSE PROCESSING (BODY -> HEADERS)
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