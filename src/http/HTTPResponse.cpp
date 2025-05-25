#include "HTTPResponse.hpp"
#include "HTTPParser.hpp"
#include <iostream>
#include <string>
#include <sys/stat.h>

HTTPResponse::HTTPResponse(void)
    : m_State(INIT), m_PollState(SOCKET_WRITE), m_CursorPos(0),
      m_ConfigServer(NULL), m_Location(NULL), m_Cgi(NULL), m_CgiFd(0), m_CgiDone(false)
{
  m_StatusCode = HTTPResponse::OK;
	m_ParseState.setReadBytes(0);
	m_ParseState.setPrevChar(0);
	m_ParseState.setState(HTTPParseState::REQ_HEADER_FIELD);
}

HTTPResponse::pollState HTTPResponse::getPollState() const {
  return m_PollState;
}

void  HTTPResponse::onHeadersParsed()
{
  long  status;
  // char *end;

  if (hasHeader("status"))
  {
    // const char *str;
    // str = getHeader("status").c_str();
    // status: 404 Not Found
    status = std::strtol(getHeader("status").c_str(), NULL, 10);
    // status = getHeader("status").c_str())
    if (status < 100 || status > 599)
    {
      std::cout << "CGI STATUS CODE IS INVALID" << std::endl;
      return ;
    }
    std::cout << "CGI STATUS: " << getHeader("status") << std::endl;
    removeHeader("status");
    // std::cout << "END: " << end << std::endl;
    // m_StatusString.assign(end);
    // std::cout << "ASSIGNED STATUS STRING : " << m_StatusString << std::endl;
    m_StatusCode = static_cast<statusCode>(status);
  }
  return;
}

void  HTTPResponse::onBodyDone()
{
  return;
}

int HTTPResponse::getCgiFd() const {
  if (!m_CgiFd)
    return -1;
  return m_CgiFd;
}

bool HTTPResponse::_isCgiPath(const std::string path,
                              const ConfigServer *configServer) {
  (void)path;
  (void)configServer;
  return false;
}

void HTTPResponse::setupCgiEnv() {
  HTTPRequest::HeaderMap headers = m_Request->getHeaders();
  this->cgi_env.clear();
  this->env_ptrs.clear();

  // std::cout << "| " << std::string(m_Request->getMethodStr()) << std::endl;
  // std::cout << "| " << m_Request->getQuery() << std::endl;
  // std::cout << "| " << m_Request->getHeader("Content-Length") << std::endl;
  // std::cout << "| " << m_Request->getHeader("Content-Type") << std::endl;

  this->cgi_env.push_back("REQUEST_METHOD=" + std::string(m_Request->getMethodStr()));
  this->cgi_env.push_back("QUERY_STRING=" + m_Request->getQuery());
  this->cgi_env.push_back("CONTENT_LENGTH=" + m_Request->getHeader("content-length"));
  this->cgi_env.push_back("CONTENT_TYPE=" + m_Request->getHeader("content-type"));
  this->cgi_env.push_back("SERVER_PROTOCOL=HTTP/1.1");

  for (HTTPRequest::HeaderMap::iterator it = headers.begin(); it != headers.end(); ++it) {
      std::string str = it->first;
      std::transform(str.begin(), str.end(), str.begin(), ::toupper);
      std::string cgi_key = "HTTP_" + str;
      // std::cout << " regex : " << cgi_key << std::endl;
      std::replace(cgi_key.begin(), cgi_key.end(), '-', '_');
      this->cgi_env.push_back(cgi_key + "=" + it->second);
  }

  for (std::vector<std::string>::iterator it = this->cgi_env.begin(); it != this->cgi_env.end(); ++it) {
      std::cout << " regex char * : " << const_cast<char*>((*it).c_str()) << std::endl;
      this->env_ptrs.push_back(const_cast<char*>((*it).c_str()));
  }
  // this->env_ptrs.push_back(NULL);
}

void HTTPResponse::_initCgi(const std::string path,
                            const CGIHandler &cgihandler,
                            const ConfigServer *configServer) {
  (void)path;
  (void)configServer;
  (void)cgihandler;

  std::string scriptName = m_Location->getScriptName(m_Request->getPath());
  std::string pathName = m_Location->getScriptPath(m_Request->getPath());
  scriptName =
      m_Location->root + m_Location->uri + m_Location->cgi_uri + scriptName;

  struct stat fileStat;
  if (stat(scriptName.c_str(), &fileStat) != 0 || !S_ISREG(fileStat.st_mode) ||
      access(scriptName.c_str(), X_OK) != 0) {
    std::cerr << "CGI script not found/executable: " << scriptName << std::endl;
    setError(NOT_FOUND);
    return;
  }

  this->setupCgiEnv();

    m_Cgi = cgihandler.spawn(pathName, scriptName);
    if (m_Cgi)
      setCgiFd(m_Cgi->cgi_sock);
    std::cout << "end init cgi" << std::endl;
    m_PollState = CGI_READ;
}

void HTTPResponse::_initBadRequest() {
  m_StatusCode = BAD_REQUEST;
  appendBody(std::string("<html><body><h1>400 Bad Request</h1></body></html>"));
  // TODO:: set headers
  addHeader("content-type", "text/html");
  addHeader("content-length", m_Body.getSize());
}

bool _safePath(const std::string path) {
  int count;
  size_t start;
  size_t i;
  std::string subpath;

  start = 0;
  count = 0;
  for (i = 0; i < path.length(); i++) {
    if (path[i] == '/') {
      if (start == i) {
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
  if (start != i) {
    subpath = path.substr(start, i - start);
    if (subpath == "..")
      count--;
    else if (subpath != ".")
      count++;
  }
  std::cout << "count: " << count << std::endl;
  return count >= 0;
}

void HTTPResponse::init(HTTPRequest const &request,
                        CGIHandler const &cgihandler,
                        ConfigServer const *configServer, int fd) {
  std::string resourcePath;
  // struct stat fileStat;

  m_ClientFd = fd;
  this->m_Request = &request;
  this->m_ConfigServer = configServer;

  m_State = PROCESS_BODY;
  // if (request.isError())
  // 	return _initBadRequest();

  // check for path traversal

  if (!_safePath(request.getPath())) {
    std::cout << "NOT A SAFE PATH" << std::endl;
    return setError(NOT_FOUND);
  }
  m_Location = &configServer->getLocation(request.getPath());
  
  if (request.isError()) {
    m_StatusCode = HTTPResponse::BAD_REQUEST;
    return setError(BAD_REQUEST);
  }

  if (m_Location->isCgiPath(request.getPath()))
    return _initCgi(request.getPath(), cgihandler, configServer);
  
  std::cout << "location root: " << m_Location->root << std::endl;
  std::cout << "location uri: " << m_Location->uri << std::endl;

  if (m_Location->uri != "/")
    resourcePath = m_Location->root + m_Location->uri + request.getPath();
  else
    resourcePath = m_Location->root + request.getPath();
  if (resourcePath[0] == '/')
    resourcePath = "." + resourcePath;
  else
    resourcePath = "./" + resourcePath;
  std::cout << "RESOURCE PATH: " << resourcePath << std::endl;
  if (_isCgiPath(request.getPath(), configServer))
    return _initCgi(request.getPath(), cgihandler, configServer);

  // ADD DEBUG INFO TO RESPONSE BODY
  // _debugBody();
  m_ResourcePath = resourcePath;
}

void HTTPResponse::reset() { return; }

bool HTTPResponse::isDone() const { return m_State == DONE; }

bool HTTPResponse::isKeepAlive() const { return false; }

bool HTTPResponse::hasCgi() const { return m_Cgi != NULL; }

HTTPResponse::statusCode HTTPResponse::getStatus() { return OK; }

HTTPResponse::responseState HTTPResponse::getState() const { return m_State; }

const std::string HTTPResponse::_statusToString() const {
  switch (m_StatusCode) {
  case 200:
    return std::string("OK");
  case 201:
    return std::string("Created");
  case 301:
    return std::string("Moved Permanently");
  case 400:
    return std::string("Bad Request");
  case 403:
    return std::string("Forbidden");
  case 404:
    return std::string("Not Found");
  case 500:
    return std::string("Internal Server Error");
  case 501:
    return std::string("Not Implemented");
  case 504:
    return std::string("Gateway Timeout");
  default:
    return std::string("UNKNOWN STATUS");
  }
}

void HTTPResponse::_sendHeaders() {
  int writtenBytes;
  size_t len;
  const char *buff;
  std::string headersString;

  headersString = m_HeadersStream.str();
  buff = headersString.c_str();
  len = headersString.length() - m_CursorPos;
  writtenBytes = send(m_ClientFd, buff + m_CursorPos, len, 0);
  std::cout << "headers size: " << headersString.length()
            << " cursorPos:" << m_CursorPos << std::endl;
  std::cout << "HEADERS: sent " << writtenBytes << " bytes to socket "
            << m_ClientFd << std::endl;
  if (writtenBytes < 0) // ERR ??
  {
    m_State = DONE;
    return;
  }
  if (m_CursorPos + writtenBytes == headersString.length()) {
    std::cout << "SEND BODY//" << std::endl;
    m_State = SEND_BODY;
    m_CursorPos = 0;
  } else
    m_CursorPos += writtenBytes;
}

void HTTPResponse::_processHeaders() {
  int statusCode;

  addHeader("content-length", m_Body.getSize());
  statusCode = static_cast<int>(m_StatusCode);
  if (m_StatusString.empty())
    m_StatusString = _statusToString();
  m_HeadersStream << "HTTP/1.1 " << statusCode << " " << m_StatusString << CRLF;
  for (HTTPRequest::HeaderMap::iterator it = m_Headers.begin();
       it != m_Headers.end(); it++) {
    m_HeadersStream << it->first << ": ";
    m_HeadersStream << it->second << CRLF;
  }
  m_HeadersStream << CRLF;
  m_State = SEND_HEADERS;
}

void HTTPResponse::_debugBody() {
  std::stringstream responseStream;

  responseStream << "<html><body>";
  responseStream << "<style>td{padding: 10;border: 2px solid black;background: "
                    "#ababed;font-size:16;font-style:italic;}</style>";
  if (m_Request->isError())
    responseStream << "<h4>PARSE ERROR</h4>";
  else
    responseStream << "<h4>PARSE SUCCESS</h4>";
  responseStream << "<h4>method: " << m_Request->getMethodStr() << "<h4>";
  responseStream << "request path: '" << m_Request->getPath() << "'"
                 << std::endl;
  responseStream << "<table>";
  responseStream << "<h1>REQUEST HEADERS</h1>";
  HTTPRequest::HeaderMap headers = m_Request->getHeaders();
  for (HTTPRequest::HeaderMap::iterator it = headers.begin();
       it != headers.end(); it++) {
    responseStream << "<tr>";
    responseStream << "<td>" << it->first << "</td>";
    responseStream << "<td>" << it->second << "</td>";
    responseStream << "</tr>";
  }
  responseStream << "</table>";
  responseStream << "</body></html>";
  responseStream << "<h1>REQUEST BODY:</h1>";
  appendBody(responseStream.str());

  appendBody(m_Request->getBody().getBuffer(), m_Request->getBody().getSize());

  appendBody(std::string("<h1>response body</h1>"));
  addHeader("content-type", "text/html");
}

const std::string HTTPResponse::_getDefaultErrorFile() const {
  std::map<HTTPResponse::statusCode, const char *> defaultPages;

  defaultPages[BAD_REQUEST] = "./html/error_400.html";
  defaultPages[NOT_FOUND] = "./html/error_404.html";
  defaultPages[FORBIDDEN] = "./html/error_403.html";
  defaultPages[SERVER_ERROR] = "./html/error_500.html";
  defaultPages[NOT_IMPLEMENTED] = "./html/error_501.html";
  defaultPages[GATEWAY_TIMEOUT] = "./html/error_504.html";

  const char * filename = defaultPages[m_StatusCode];
  if (!filename)
  {
    std::cout << "CANNOT FIND DEFAULT ERROR PAGE" << std::endl;
    return std::string(defaultPages[SERVER_ERROR]);
  }
  return std::string(filename);
}

bool HTTPResponse::_validFile(const std::string filename) const {
  struct stat fileStat;

  if (access(filename.c_str(), R_OK) != 0 ||
      stat(filename.c_str(), &fileStat) == -1)
    return false;
  return S_ISREG(fileStat.st_mode);
}

bool HTTPResponse::_validDirectory(const std::string filename) const {
  struct stat fileStat;

  if (access(filename.c_str(), R_OK | X_OK) != 0 ||
      stat(filename.c_str(), &fileStat) == -1)
    return false;
  return S_ISDIR(fileStat.st_mode);
}

void HTTPResponse::_readFileToBody(const std::string filename) {
  std::ifstream fileStream;
  char buff[4096];

  fileStream.open(filename.c_str());
  std::streamsize rbytes = 4096;
  while (rbytes >= 4096) {
    fileStream.read(buff, 4096);
    rbytes = fileStream.gcount();
    std::cout << "read " << rbytes << " bytes from file " << std::endl;
    appendBody(buff, rbytes);
  }
}

void HTTPResponse::_processErrorBody()
{
  std::cout << "PROCESS ERROR BODY" << std::endl;
  const std::map<int, std::string> &errors = m_ConfigServer->errors;
  std::map<int, std::string>::const_iterator it;
  std::string filename;
  std::cout << "AFTER -----" << std::endl;

  it = errors.find(static_cast<int>(m_StatusCode));
  if (it == errors.end())
  {
    std::cout << "Cant find config error page" << std::endl;
    filename = _getDefaultErrorFile();
  }
  else
  {
    std::cout << "[RESPONSE] Found config error page" << std::endl;
    if (!m_Location)
    {
      std::cout << "NO LOCATION" << std::endl;
    }
    filename = "./" + m_Location->root + "/" + it->second;
  }
  std::cout << "FILENAME: " << filename << std::endl;
  if (!_validFile(filename))
  {
    std::string body;
    body = "<html><body><h1>Unexpected error";
    body += "</h1><p>(Default WebServ Error Page)</body></html>";
    appendBody(body);
    return;
  } else
    _readFileToBody(filename);
}

// PATH TRAVERSAL
void HTTPResponse::_processDirectoryListing() {
  DIR *dir;
  struct dirent *entry;
  std::string path;

  if (!m_Location->autoindex) {
    m_StatusCode = FORBIDDEN;
    return _processErrorBody();
  }
  std::cout << "opening dir:" << m_ResourcePath << std::endl;
  dir = opendir(m_ResourcePath.c_str());
  if (!dir)
    return setError(SERVER_ERROR);
  std::cout << "DIRECTORY LISTING" << std::endl;
  std::stringstream body;
  path = m_Request->getPath();
  if (path[path.length() - 1] != '/')
    path += "/";

  body << "<html><title>" << m_Request->getPath() << " listing"
       << "</title><body>";
  body << "<h1>" << path << " listing</h1><ul>";
  entry = readdir(dir);
  while (entry) {
    body << "<li><a href=\"" << path << entry->d_name << "\">" << entry->d_name
         << "</a></li>";
    entry = readdir(dir);
  }
  body << "</ul></body></html>";
  appendBody(body.str());
  closedir(dir);
}

void HTTPResponse::setError(statusCode status) {
  m_Body.clear();
  std::cout << "[RESPONSE] set error: " << status << std::endl;
  m_StatusCode = status;
  _processErrorBody();
  m_State = PROCESS_HEADERS;
  m_PollState = SOCKET_WRITE;
}

void HTTPResponse::_processResource() {
  if (m_Request->getMethod() != GET)
    return setError(NOT_IMPLEMENTED);
  if (_validFile(m_ResourcePath))
    return;
  if (_validDirectory(m_ResourcePath)) {
    if (m_ResourcePath[m_ResourcePath.length() - 1] != '/') {
      m_StatusCode = MOVED_PERMANENTLY;
      addHeader("location", m_Request->getPath() + "/");
      return;
    }
    std::string indexPath = m_ResourcePath + "index.html";
    if (_validFile(indexPath)) {
      m_ResourcePath = indexPath;
      return;
    } else if (m_Location->autoindex)
      _processDirectoryListing();
    else
      setError(NOT_FOUND);
  } else
    setError(NOT_FOUND);
}

void HTTPResponse::_processCgiBody() {
  
  std::cout << "PROCESS CGI BODY" << std::endl;
  char buff[8192];
  ssize_t rbytes = m_Cgi->read(buff, 8192);
  // buff[rbytes] = 0;
  // std::cout << buff << std::endl;
  // std::cout << "read from cgi: " << rbytes  << " | " << buff << std::endl;
  if (rbytes > 0)
    HTTPParser::parseCgi(*this, buff, rbytes);

  if (rbytes < 8192 && m_CgiDone)
  {
    std::cout << "CGI READ FINISHED" << std::endl;
    m_PollState = SOCKET_WRITE;
    m_State = PROCESS_HEADERS;
  }
  // setError(SERVER_ERROR);
  // m_State = PROCESS_HEADERS;
  // m_PollState = SOCKET_WRITE;
}

void HTTPResponse::_processBody() {
  _processResource();
  if (m_StatusCode == OK)
    _readFileToBody(m_ResourcePath);
  m_State = PROCESS_HEADERS;
}

void HTTPResponse::_sendBody() {
  std::cout << "PROCESS BODY" << std::endl;
  const char *bodyBuffer = m_Body.getBuffer();
  // const char * buff = const_cast<const char *>(&bodyBuffer[m_CursorPos]);
  std::size_t size = m_Body.getSize();
  // write(1, buff, size);

  ssize_t wBytes = send(m_ClientFd, bodyBuffer, size, 0);
  if (wBytes < 0) {
    m_State = DONE;
    return;
  }
  std::cout << "body size: " << m_Body.getSize() << std::endl;
  std::cout << "BODY: sent " << wBytes << " bytes to socket " << m_ClientFd
            << std::endl;
  if (static_cast<size_t>(wBytes) < size)
    std::cout << "written: " << wBytes << std::endl;
  else
    m_State = DONE;
}

bool HTTPResponse::resume(bool isCgiReady, bool isClientReady) {
  std::cout << "ENTER RESUME cgi: " << isCgiReady << " client: " << isClientReady << std::endl;
  if (isCgiReady)
    std::cout << "CGI READY" << std::endl;
  if (isClientReady)
    std::cout << "CLIENT READY" << std::endl;

  // RESPONSE PROCESSING (BODY -> HEADERS)
  if (m_PollState == CGI_READ && isCgiReady)
    return (_processCgiBody(), false);
  
  if (!isClientReady)
    return false;
  else if (!hasCgi() && m_State == PROCESS_BODY)
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

// HTTPResponse::~HTTPResponse(void) {}
HTTPResponse::~HTTPResponse() {
  if (m_Cgi)
    m_Cgi->cleanup(false);
}
