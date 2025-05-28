#include "HTTPResponse.hpp"
#include "HTTPParser.hpp"
#include <cstdlib>   // for realpath
#include <iostream>  // for std::cerr
#include <string>
#include "Resource.hpp"

HTTPResponse::status_map_t  createDefaultPages()
{
  	HTTPResponse::status_map_t defaultPages;

    defaultPages[HTTPResponse::BAD_REQUEST] = "./html/error_400.html";
    defaultPages[HTTPResponse::NOT_FOUND] = "./html/error_404.html";
    defaultPages[HTTPResponse::METHOD_NOT_ALLOWED] = "./html/error_405.html";
    defaultPages[HTTPResponse::FORBIDDEN] = "./html/error_403.html";
    defaultPages[HTTPResponse::SERVER_ERROR] = "./html/error_500.html";
    defaultPages[HTTPResponse::NOT_IMPLEMENTED] = "./html/error_501.html";
    defaultPages[HTTPResponse::GATEWAY_TIMEOUT] = "./html/error_504.html";

    return defaultPages;
}


HTTPResponse::status_map_t  createStatusMap()
{
  HTTPResponse::status_map_t statusString;

  statusString[HTTPResponse::OK] = "OK";
  statusString[HTTPResponse::CREATED]= "CREATED";
  statusString[HTTPResponse::NO_CONTENT]= "No Content";
  statusString[HTTPResponse::BAD_REQUEST]= "Bad Request";
  statusString[HTTPResponse::NOT_FOUND]= "Not Found";
  statusString[HTTPResponse::METHOD_NOT_ALLOWED]= "Method Not Allowed";
  statusString[HTTPResponse::FORBIDDEN]= "Forbidden";
  statusString[HTTPResponse::SERVER_ERROR]= "Internal Server Error";
  statusString[HTTPResponse::NOT_IMPLEMENTED]= "Not Implemented";
  statusString[HTTPResponse::GATEWAY_TIMEOUT]= "Gateway Timeout";
  statusString[HTTPResponse::MOVED_PERMANENTLY]= "Moved Permanently";

  return statusString;
}

const HTTPResponse::status_map_t HTTPResponse::_defaultPages = createDefaultPages();
const HTTPResponse::status_map_t HTTPResponse::_statusMap = createStatusMap();

HTTPResponse::HTTPResponse(void)
    : m_State(INIT), m_PollState(SOCKET_WRITE), m_CursorPos(0),
      m_ConfigServer(NULL), m_Location(NULL), m_Cgi(NULL), m_CgiFd(0), m_CgiDone(false), m_HasCgi(false)
{
  m_StatusCode = HTTPResponse::OK;
	m_ParseState.setReadBytes(0);
	m_ParseState.setPrevChar(0);
	m_ParseState.setState(HTTPParseState::PARSE_HEADER_FIELD);
}

HTTPResponse::pollState HTTPResponse::getPollState() const {
  return m_PollState;
}

void  HTTPResponse::onHeadersParsed()
{

  if (hasHeader("status"))
  {
    std::string statusHeader = getHeader("status");
    long        status;
    char        *end;

    status = std::strtol(statusHeader.c_str(), &end, 10);
    if (status < 100 || status > 599)
    {
      std::cout << "CGI STATUS CODE IS INVALID" << std::endl;
      return ;
    }
    while (*end == SP)
      end++;
    if (end)
      m_StatusString.assign(end);
    std::cout << "CGI STATUS: " << statusHeader << std::endl;
    removeHeader("status");
    std::cout << "END: " << end << std::endl;
    // m_StatusString.assign(end);
    // std::cout << "ASSIGNED STATUS STRING : " << m_StatusString << std::endl;
    m_StatusCode = static_cast<statusCode>(status);
  }
  return;
}

void  HTTPResponse::onBodyDone()
{
  m_CgiDone = true;
  m_ParseState.setState(HTTPParseState::PARSE_DONE);
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


std::string getAbsolutePath(const std::string& relativePath) {
    char resolvedPath[PATH_MAX];
    if (realpath(relativePath.c_str(), resolvedPath)) {
        return std::string(resolvedPath);
    } else {
        perror("realpath");
        return "";
    }
}


void HTTPResponse::setupCgiEnv(std::string &ScriptFileName) {
  HTTPRequest::header_map_t headers = m_Request->getHeaders();
  this->cgi_env.clear();
  this->env_ptrs.clear();

  // std::cout << "| " << std::string(m_Request->getMethodStr()) << std::endl;
  // std::cout << "| " << m_Request->getQuery() << std::endl;
  // std::cout << "| " << m_Request->getHeader("Content-Length") << std::endl;
  // std::cout << "| " << m_Request->getHeader("Content-Type") << std::endl;

  for (int i = 0; environ[i]; i++)
  {
      this->cgi_env.push_back(std::string(environ[i]));
  }

  for (HTTPRequest::header_map_t::iterator it = headers.begin(); it != headers.end(); ++it) {
      std::string str = it->first;
      std::transform(str.begin(), str.end(), str.begin(), ::toupper);
      std::string cgi_key = "HTTP_" + str;
      // std::cout << " regex : " << cgi_key << std::endl;
      std::replace(cgi_key.begin(), cgi_key.end(), '-', '_');
      this->cgi_env.push_back(cgi_key + "=" + it->second);
  }
  std::string scriptAbsolute = getAbsolutePath(ScriptFileName);

  if (!scriptAbsolute.empty()){
    this->cgi_env.push_back("SCRIPT_FILENAME=" + scriptAbsolute);
    this->cgi_env.push_back("PATH_TRANSLATED=" + scriptAbsolute);
  }
  else {
    this->cgi_env.push_back("SCRIPT_FILENAME=" + ScriptFileName);
    this->cgi_env.push_back("PATH_TRANSLATED=" + ScriptFileName);
  }

  this->cgi_env.push_back("REQUEST_METHOD=" + std::string(m_Request->getMethodStr()));
  this->cgi_env.push_back("SCRIPT_NAME=" + std::string(m_Request->getPath()));
  this->cgi_env.push_back("QUERY_STRING=" + m_Request->getQuery());
  this->cgi_env.push_back("PATH_INFO=" + m_Request->getPath());
  // Temporary ??
  if (m_Request->hasHeader("content-length"))
    this->cgi_env.push_back("CONTENT_LENGTH=" + m_Request->getHeader("content-length"));
  if (m_Request->hasHeader("content-type"))
    this->cgi_env.push_back("CONTENT_TYPE=" + m_Request->getHeader("content-type"));
  this->cgi_env.push_back("SERVER_PROTOCOL=HTTP/1.1");
  this->cgi_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
  this->cgi_env.push_back("REDIRECT_STATUS=200");

  this->cgi_env.push_back("REQUEST_PATH=" + m_Request->getPath());
  this->cgi_env.push_back("REQUEST_URI=" + m_Request->getUri());
  // this->cgi_env.push_back("REMOTE_ADDR=127.0.0.1");
  // this->cgi_env.push_back("REMOTE_PORT=4000");
  // setenv("REQUEST_URI", "/orog?name=value&foo=bar", 1);  // From URL after ?
  // setenv("DOCUMENT_ROOT", "/home/bchanaa/Desktop/webserv/www/", 1);


  // this->cgi_env.push_back("SCRIPT_NAME=hello.php");
  // this->cgi_env.push_back("REQUEST_METHOD=");

  for (std::vector<std::string>::iterator it = this->cgi_env.begin(); it != this->cgi_env.end(); ++it) {
      std::cout << " regex char * : " << const_cast<char*>((*it).c_str()) << std::endl;
      this->env_ptrs.push_back(const_cast<char*>((*it).c_str()));
  }
  this->env_ptrs.push_back(NULL);
  // this->env_ptrs.push_back(NULL);
}

void HTTPResponse::_initCgi(const std::string path,
                            const CGIHandler &cgihandler,
                            const ConfigServer *configServer)
{
  (void)path;
  (void)configServer;
  (void)cgihandler;

  std::string scriptName = m_Location->getScriptName(m_Request->getPath());
  std::string pathName = m_Location->getScriptPath(m_Request->getPath());
  scriptName =
      m_Location->root + m_Location->uri + m_Location->cgi_uri + scriptName;
  
  std::cout << "scriptName : " << scriptName << " | " << pathName << std::endl;

  struct stat fileStat;
  if (stat(scriptName.c_str(), &fileStat) != 0 || !S_ISREG(fileStat.st_mode) ||
      access(scriptName.c_str(), X_OK) != 0) {
    std::cerr << "CGI script not found/executable: " << scriptName << std::endl;
    setError(NOT_FOUND);
    return;
  }

  this->setupCgiEnv(scriptName);

    m_Cgi = cgihandler.spawn(pathName, scriptName, static_cast<char **>(&this->env_ptrs[0]));
    if (m_Cgi)
    {
      m_HasCgi = true;
      setCgiFd(m_Cgi->cgi_sock);
    }
    std::cout << "end init cgi" << std::endl;
    if (m_Request->getBody().getSize() > 0)
      m_PollState = CGI_WRITE;
    else
      m_PollState = CGI_READ;
    _debugBody();
}

bool _safePath(const std::string path) {
  int count;
  size_t  start;
  size_t  i;
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

void  HTTPResponse::_normalizeResourcePath()
{
  const std::string &requestPath = m_Request->getPath();

  if (m_Location->uri != "/")
    m_ResourcePath = m_Location->root + m_Location->uri + requestPath;
  else
    m_ResourcePath = m_Location->root + requestPath;
  if (m_ResourcePath[0] == '/')
    m_ResourcePath = "." + m_ResourcePath;
  else
    m_ResourcePath = "./" + m_ResourcePath;
  // m_ResourcePath.insert(0, "./")
}

void HTTPResponse::init(HTTPRequest &request,
                        CGIHandler const &cgihandler,
                        ConfigServer const *configServer, int fd) {
  std::string resourcePath;

  m_ClientFd = fd;
  this->m_Request = &request;
  this->m_ConfigServer = configServer;

  m_State = PROCESS_BODY;

  // check for path traversal
  if (!_safePath(request.getPath())) {
    std::cout << "NOT A SAFE PATH" << std::endl;
    return setError(NOT_FOUND);
  }
  m_Location = &configServer->getLocation(request.getPath());
  
  if (request.isError())
    return setError(BAD_REQUEST);

  if ( not m_Location->isMethodAllowed(request.getMethod()))
  {

    std::cout << "METHOD NOT ALLOWED" << std::endl;
    return setError(METHOD_NOT_ALLOWED);
  }

  if (m_Location->isCgiPath(request.getPath()))
    return _initCgi(request.getPath(), cgihandler, configServer);
  
  std::cout << "location root: " << m_Location->root << std::endl;
  std::cout << "location uri: " << m_Location->uri << std::endl;

  std::cout << "RESOURCE PATH: " << resourcePath << std::endl;
  if (_isCgiPath(request.getPath(), configServer))
    return _initCgi(request.getPath(), cgihandler, configServer);

  // ADD DEBUG INFO TO RESPONSE BODY
  // _debugBody();
  m_ResourcePath = resourcePath;
  _normalizeResourcePath();

  httpMethod requestMethod = request.getMethod();
  if (requestMethod == DELETE)
    _handleDeleteMethod();
}

void HTTPResponse::reset() { return; }

bool HTTPResponse::isDone() const { return m_State == DONE; }

bool HTTPResponse::isKeepAlive() const { return false; }

bool HTTPResponse::hasCgi() const { return m_HasCgi; }

HTTPResponse::statusCode HTTPResponse::getStatus() { return OK; }

HTTPResponse::responseState HTTPResponse::getState() const { return m_State; }

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
  {
    status_map_t::const_iterator it = _statusMap.find(m_StatusCode);
    if (it != _statusMap.end())
      m_StatusString = it->second;
    else
      m_StatusString = "UNKNOWN STATUS";
  }
  m_HeadersStream << "HTTP/1.1 " << statusCode << " " << m_StatusString << CRLF;
  for (HTTPRequest::header_map_t::iterator it = m_Headers.begin();
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
  responseStream << "request query: '" << m_Request->getQuery() << "'"
                 << std::endl;
  responseStream << "<table>";
  responseStream << "<h1>REQUEST HEADERS</h1>";
  HTTPRequest::header_map_t headers = m_Request->getHeaders();
  for (HTTPRequest::header_map_t::iterator it = headers.begin();
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

  status_map_t::const_iterator it = _defaultPages.find(m_StatusCode);
  if (it != _defaultPages.end())
    return std::string(it->second);
  
  std::cout << "CANNOT FIND DEFAULT ERROR PAGE" << std::endl;
  return std::string(_defaultPages.find(SERVER_ERROR)->second);
}

// bool HTTPResponse::_validFile(const std::string filename) const {
//   struct stat fileStat;

//   if (access(filename.c_str(), R_OK) != 0 ||
//       stat(filename.c_str(), &fileStat) == -1)
//     return false;
//   return S_ISREG(fileStat.st_mode);
// }

// bool HTTPResponse::_validDirectory(const std::string filename) const {
//   struct stat fileStat;

//   if (access(filename.c_str(), R_OK | X_OK) != 0 ||
//       stat(filename.c_str(), &fileStat) == -1)
//     return false;
//   return S_ISDIR(fileStat.st_mode);
// }

void HTTPResponse::_readFileToBody(const std::string filename) {
  std::ifstream fileStream;
  char buff[4096];

  std::cout << "READING FILE: " << filename << std::endl;
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
    filename = "./" + m_Location->root + "/" + it->second;
  }
  std::cout << "FILENAME: " << filename << std::endl;
  Resource file(filename.c_str());
  if (!file.exists() || !file.canRead())
  {
    std::string body;
    body = "<html><body><h1>Unexpected error";
    body += "</h1><p>(Default WebServ Error Page)</body></html>";
    appendBody(body);
    return;
  } else
    _readFileToBody(filename);
}

void  HTTPResponse::_handleDeleteMethod()
{
  Resource resource(m_ResourcePath.c_str());
  bool  removed = resource.remove();

  if (removed)
    m_StatusCode = NO_CONTENT;
  else
  {
    setError(NOT_FOUND);
    std::cout << "COULD NOT REMOVE RESOURCE" << std::endl;
  }
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

  Resource resource(m_ResourcePath.c_str());
  
  if (resource.isFile() && resource.canRead())
    return;
  if (resource.validDirectory()) {
    if (m_ResourcePath[m_ResourcePath.length() - 1] != '/') {
      m_StatusCode = MOVED_PERMANENTLY;
      addHeader("location", m_Request->getPath() + "/");
      return;
    }
    std::string indexPath = m_ResourcePath + "index.html";
    resource = indexPath.c_str();
    if (resource.isFile() && resource.canRead()) {
      m_ResourcePath = indexPath;
      return;
    } else if (m_Location->autoindex)
      _processDirectoryListing();
    else
      setError(FORBIDDEN);
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
    // appendBody(buff, rbytes);

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

void HTTPResponse::_writeToCgi()
{
  if (m_CgiDone)
  {
    m_State = PROCESS_HEADERS;
    m_PollState = SOCKET_WRITE;
    return ;
  }
  HTTPBody &body = m_Request->getBody();
  ssize_t sent = m_Cgi->write(body);
  std::cout << "WROTE " << sent << " bytes to CGI" << std::endl;
  if (body.getSize() == 0)
    m_PollState = CGI_READ;
}

void HTTPResponse::_sendBody() {
  std::cout << "PROCESS BODY" << std::endl;
  const char *bodyBuffer = m_Body.getBuffer();
  // const char * buff = const_cast<const char *>(&bodyBuffer[m_CursorPos]);
  std::size_t size = m_Body.getSize();
  // write(1, buff, size);

  ssize_t wBytes = send(m_ClientFd, bodyBuffer, size, 0);
  if (wBytes < 0)
  {
    std::cout << "[RESPONSE ERROR]: could not send response to client." << std::endl;
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
  if (m_PollState == CGI_WRITE && isCgiReady)
    return (_writeToCgi(), false);
  else if (m_PollState == CGI_READ && isCgiReady)
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
