#include "../../includes/HTTPResponse.hpp"
#include "../../includes/Constants.hpp"
#include "../../includes/HTTPParser.hpp"
#include "../../includes/Resource.hpp"
#include <cstdlib>  // for realpath
#include <iostream> // for std::cerr
#include <string>

int HTTPResponse::getCgiStderrFd() const {
  if (!m_Cgi)
    return -1;
  return m_Cgi->cgi_stderr_sock;
}

void HTTPResponse::processStderr() {
  if (!m_Cgi)
    return;

  char stderrBuff[READ_BUFFER_SIZE];
  ssize_t rbytes = m_Cgi->readStderr(stderrBuff, READ_BUFFER_SIZE);

  if (rbytes > 0) {
    m_StderrBuffer.append(stderrBuff, rbytes);
    m_HasStderrData = true;
    std::cout << "[CGI STDERR]: " << std::string(stderrBuff, rbytes)
              << std::endl;
  }
}

HTTPResponse::status_map_t createDefaultPages() {
  HTTPResponse::status_map_t defaultPages;

  defaultPages[HTTPResponse::BAD_REQUEST] = "./html/error_400.html";
  defaultPages[HTTPResponse::NOT_FOUND] = "./html/error_404.html";
  defaultPages[HTTPResponse::METHOD_NOT_ALLOWED] = "./html/error_405.html";
  defaultPages[HTTPResponse::FORBIDDEN] = "./html/error_403.html";
  defaultPages[HTTPResponse::CONTENT_TOO_LARGE] = "./html/error_413.html";
  defaultPages[HTTPResponse::SERVER_ERROR] = "./html/error_500.html";
  defaultPages[HTTPResponse::NOT_IMPLEMENTED] = "./html/error_501.html";
  defaultPages[HTTPResponse::GATEWAY_TIMEOUT] = "./html/error_504.html";

  return defaultPages;
}

HTTPResponse::status_map_t createStatusMap() {
  HTTPResponse::status_map_t statusString;

  statusString[HTTPResponse::OK] = "OK";
  statusString[HTTPResponse::CREATED] = "CREATED";
  statusString[HTTPResponse::NO_CONTENT] = "No Content";
  statusString[HTTPResponse::BAD_REQUEST] = "Bad Request";
  statusString[HTTPResponse::NOT_FOUND] = "Not Found";
  statusString[HTTPResponse::METHOD_NOT_ALLOWED] = "Method Not Allowed";
  statusString[HTTPResponse::FORBIDDEN] = "Forbidden";
  statusString[HTTPResponse::SERVER_ERROR] = "Internal Server Error";
  statusString[HTTPResponse::NOT_IMPLEMENTED] = "Not Implemented";
  statusString[HTTPResponse::GATEWAY_TIMEOUT] = "Gateway Timeout";
  statusString[HTTPResponse::MOVED_PERMANENTLY] = "Moved Permanently";
  statusString[HTTPResponse::CONTENT_TOO_LARGE] = "Content Too Large";

  return statusString;
}

const HTTPResponse::status_map_t HTTPResponse::_defaultPages =
    createDefaultPages();
const HTTPResponse::status_map_t HTTPResponse::_statusMap = createStatusMap();

HTTPResponse::HTTPResponse(void)
    : m_SocketBuffer(READ_BUFFER_SIZE), m_State(INIT),
      m_PollState(SOCKET_WRITE), m_CursorPos(0), m_ConfigServer(NULL),
      m_Location(NULL), m_Cgi(NULL), m_CgiFd(0), m_CgiDone(false),
      m_HasCgi(false), m_HasStderrData(false) {
  addHeader("server", WEBSERVER_NAME);
  m_StatusCode = HTTPResponse::OK;
  m_ParseState.setReadBytes(0);
  m_ParseState.setPrevChar(0);
  m_ParseState.setState(HTTPParseState::PARSE_HEADER_FIELD);
}

HTTPResponse::pollState HTTPResponse::getPollState() const {
  return m_PollState;
}

void HTTPResponse::addHeader(std::string key, std::string value) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  if (key == "set-cookie") {
    m_Cookies.push_back(value);
  } else
    HTTPHeaders::addHeader(key, value);
}

void HTTPResponse::onHeadersParsed() {

  if (hasHeader("status")) {
    std::string statusHeader = getHeader("status");
    long status;
    char *end;

    status = std::strtol(statusHeader.c_str(), &end, 10);
    if (status < 100 || status > 599) {
      std::cout << "CGI STATUS CODE IS INVALID" << std::endl;
      return;
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

void HTTPResponse::onBodyDone() {
  m_Body.seal();
  m_CgiDone = true;
  m_ParseState.setState(HTTPParseState::PARSE_DONE);
  return;
}

int HTTPResponse::getCgiFd() const {
  if (!m_CgiFd)
    return -1;
  return m_CgiFd;
}

// bool HTTPResponse::_isCgiPath(const std::string path,
//                               const ConfigServer *configServer) {
//   (void)path;
//   (void)configServer;
//   return false;
// }

std::string getAbsolutePath(const std::string &relativePath) {
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

  for (int i = 0; environ[i]; i++) {
    this->cgi_env.push_back(std::string(environ[i]));
  }

  for (HTTPRequest::header_map_t::iterator it = headers.begin();
       it != headers.end(); ++it) {
    std::string str = it->first;
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    std::string cgi_key = "HTTP_" + str;
    // std::cout << " regex : " << cgi_key << std::endl;
    std::replace(cgi_key.begin(), cgi_key.end(), '-', '_');
    this->cgi_env.push_back(cgi_key + "=" + it->second);
  }
  std::string scriptAbsolute = getAbsolutePath(ScriptFileName);

  if (!scriptAbsolute.empty()) {
    this->cgi_env.push_back("SCRIPT_FILENAME=" + scriptAbsolute);
    this->cgi_env.push_back("PATH_TRANSLATED=" + scriptAbsolute);
  } else {
    this->cgi_env.push_back("SCRIPT_FILENAME=" + ScriptFileName);
    this->cgi_env.push_back("PATH_TRANSLATED=" + ScriptFileName);
  }

  this->cgi_env.push_back("REQUEST_METHOD=" +
                          std::string(m_Request->getMethodStr()));
  this->cgi_env.push_back("SCRIPT_NAME=" + std::string(m_Request->getPath()));
  this->cgi_env.push_back("QUERY_STRING=" + m_Request->getQuery());
  this->cgi_env.push_back("PATH_INFO=" + m_Request->getPath());
  // Temporary ??
  if (m_Request->hasHeader("content-length"))
    this->cgi_env.push_back("CONTENT_LENGTH=" +
                            m_Request->getHeader("content-length"));
  if (m_Request->hasHeader("content-type"))
    this->cgi_env.push_back("CONTENT_TYPE=" +
                            m_Request->getHeader("content-type"));
  this->cgi_env.push_back("SERVER_PROTOCOL=HTTP/1.1");
  this->cgi_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
  this->cgi_env.push_back("REDIRECT_STATUS=200");

  this->cgi_env.push_back("REQUEST_PATH=" + m_Request->getPath());
  this->cgi_env.push_back("REQUEST_URI=" + m_Request->getUri());

  for (std::vector<std::string>::iterator it = this->cgi_env.begin();
       it != this->cgi_env.end(); ++it) {
    // std::cout << " regex char * : " << const_cast<char*>((*it).c_str()) <<
    // std::endl;
    this->env_ptrs.push_back(const_cast<char *>((*it).c_str()));
  }
  this->env_ptrs.push_back(NULL);
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

  this->setupCgiEnv(scriptName);

  m_Cgi = cgihandler.spawn(pathName, scriptName,
                           static_cast<char **>(&this->env_ptrs[0]));
  if (m_Cgi) {
    m_HasCgi = true;
    setCgiFd(m_Cgi->cgi_stdout_sock);
  }
  if (m_Request->getBody().getSize() > 0)
    m_PollState = CGI_WRITE;
  else
    m_PollState = CGI_READ;
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
  return count >= 0;
}

void HTTPResponse::_normalizeResourcePath() {
  const std::string &requestPath = m_Request->getPath();

  m_ResourcePath = m_Location->root + requestPath;
  if (m_ResourcePath[0] == '/')
    m_ResourcePath = "." + m_ResourcePath;
  else
    m_ResourcePath = "./" + m_ResourcePath;
  // m_ResourcePath.insert(0, "./")
}

void HTTPResponse::init(HTTPRequest &request, CGIHandler const &cgihandler,
                        ConfigServer const *configServer, int fd) {
  std::string resourcePath;
  m_ClientFd = fd;
  this->m_Request = &request;
  this->m_ConfigServer = configServer;
  m_State = PROCESS_BODY;

  // check for path traversal
  if (!_safePath(request.getPath()))
    return setError(NOT_FOUND);
  m_Location = configServer->getLocation(request.getPath());

  if (request.isError()) {
    switch (request.getError()) {
    case ERR_CONTENT_TOO_LARGE:
      return setError(CONTENT_TOO_LARGE);
    default:
      return setError(BAD_REQUEST);
    }
  }
  if (not m_Location->isMethodAllowed(request.getMethod()))
    return setError(METHOD_NOT_ALLOWED);
  if (m_Location->isCgiPath(request.getPath()))
    return _initCgi(request.getPath(), cgihandler, configServer);
  m_ResourcePath = resourcePath;
  _normalizeResourcePath();
  httpMethod requestMethod = request.getMethod();
  if (requestMethod == DELETE)
    _handleDeleteMethod();

  if (requestMethod == POST || requestMethod == PUT)
    _handleFileUpload();
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
  if (writtenBytes < 0) {
    m_State = DONE;
    return;
  }
  if (m_CursorPos + writtenBytes == headersString.length()) {
    m_State = SEND_BODY;
    m_CursorPos = 0;
  } else
    m_CursorPos += writtenBytes;
}

void HTTPResponse::_processHeaders() {
  int statusCode;

  if (m_Body.getSize() > 0)
    HTTPHeaders::addHeader("content-length", m_Body.getSize());
  statusCode = static_cast<int>(m_StatusCode);
  if (m_StatusString.empty()) {
    status_map_t::const_iterator it = _statusMap.find(m_StatusCode);
    if (it != _statusMap.end())
      m_StatusString = it->second;
    else
      m_StatusString = "UNKNOWN";
  }
  m_HeadersStream << "HTTP/1.1 " << statusCode << " " << m_StatusString << CRLF;
  std::cout << "[RESPONSE] : " << m_HeadersStream.str() << std::endl;
  for (HTTPRequest::header_map_t::iterator it = m_Headers.begin();
       it != m_Headers.end(); it++) {
    m_HeadersStream << it->first << ": ";
    m_HeadersStream << it->second << CRLF;
  }
  for (std::vector<std::string>::const_iterator it = m_Cookies.begin();
       it != m_Cookies.end(); ++it) {
    m_HeadersStream << "Set-Cookie" << ": ";
    m_HeadersStream << *it << CRLF;
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

  // appendBody(m_Request->getBody().getBuffer(),
  // m_Request->getBody().getSize());
  // TODO: appendBody(httpbody)

  appendBody(std::string("<h1>response body</h1>"));
  HTTPHeaders::addHeader("content-type", "text/html");
}

const std::string HTTPResponse::_getDefaultErrorFile() const {

  status_map_t::const_iterator it = _defaultPages.find(m_StatusCode);
  if (it != _defaultPages.end())
    return std::string(it->second);

  return std::string(_defaultPages.find(SERVER_ERROR)->second);
}

void HTTPResponse::_readFileToBody(const std::string filename) {
  std::ifstream fileStream;
  char buff[READ_BUFFER_SIZE];

  std::cout << "[REQUEST] reading file: " << filename << std::endl;
  fileStream.open(filename.c_str());
  std::streamsize rbytes = READ_BUFFER_SIZE;
  while (rbytes >= READ_BUFFER_SIZE) {
    fileStream.read(buff, READ_BUFFER_SIZE);
    rbytes = fileStream.gcount();
    appendBody(buff, rbytes);
  }
}

void HTTPResponse::_processErrorBody() {
  const std::map<int, std::string> &errors = m_ConfigServer->errors;
  std::map<int, std::string>::const_iterator it;
  std::string filename;

  it = errors.find(static_cast<int>(m_StatusCode));
  if (it == errors.end())
    filename = _getDefaultErrorFile();
  else
    filename = "./" + m_Location->root + "/" + it->second;
  Resource file(filename.c_str());
  if (!file.exists() || !file.canRead()) {
    std::string body;
    body = "<html><body><h1>Error</h1>";
    body += "<p>Unexpected error occured -- (Default WebServ Error "
            "Page)</p></body></html>";
    appendBody(body);
    return;
  } else
    _readFileToBody(filename);
}

HTTPResponse::statusCode saveToFile(std::string filename, HTTPBody &body) {
  std::ofstream file;
  std::cout << "SAVE TO FILE: " << filename << std::endl;

  file.open(filename.c_str(), std::ios::out | std::ios::trunc);
  if (!file.is_open())
    return (HTTPResponse::FORBIDDEN);
  char buff[READ_BUFFER_SIZE];
  while (body.getSize() > 0) {
    size_t rbytes = body.read(buff, READ_BUFFER_SIZE);
    file.write(buff, rbytes);
    if (file.fail())
      return (HTTPResponse::SERVER_ERROR);
  }
  return (HTTPResponse::CREATED);
}

std::string getFilename(const std::string &path) {
  size_t pos = path.find_last_of("/");
  if (pos == std::string::npos)
    return path;
  return path.substr(pos + 1);
}

std::string joinPath(std::string base, std::string name) {
  std::string path(base);
  if (name[0] != '/' && base[base.length() - 1] != '/')
    path += "/";
  path += name;
  return path;
}

void HTTPResponse::_handleFileUpload() {
  if (m_Request->getMethod() == PUT) {
    if (m_Request->isMultipartForm())
      return setError(BAD_REQUEST);
    std::string basePath = joinPath(m_Location->root, m_Location->upload);
    std::string filename = getFilename(m_ResourcePath);
    if (filename[0] == '/')
      filename = "." + filename;
    statusCode status =
        saveToFile(joinPath(basePath, filename), m_Request->getBody());
    addHeader("Location", filename);
    m_Body.seal();
    m_State = PROCESS_HEADERS;
    m_PollState = SOCKET_WRITE;
    m_StatusCode = status;
    return;
  }
  if (!m_Request->isMultipartForm())
    return setError(NOT_IMPLEMENTED);
  std::vector<FormPart *> &formParts = m_Request->multipartForm->getParts();
  if (formParts.size() < 1)
    return setError(BAD_REQUEST);
  std::cout << "part count " << formParts.size() << std::endl;
  FormPart *filePart = m_Request->multipartForm->getFirstFilePart();
  if (!filePart)
    return setError(BAD_REQUEST);
  HTTPHeaders::header_map_t directives = filePart->getDispositionDirectives();

  std::string basePath = joinPath(m_Location->root, m_Location->upload);
  std::string filename =
      joinPath(basePath, removeQuotes(directives["filename"]));
  if (filename[0] == '/')
    filename = "." + filename;
  statusCode status = saveToFile(filename, filePart->getBody());
  addHeader("Location", filename);
  m_Body.seal();
  m_State = PROCESS_HEADERS;
  m_PollState = SOCKET_WRITE;
  m_StatusCode = status;
  return;
}

void HTTPResponse::_handleDeleteMethod() {
  Resource resource(m_ResourcePath.c_str());
  bool removed = resource.remove();

  if (removed) {
    m_State = PROCESS_HEADERS;
    m_PollState = SOCKET_WRITE;
    m_StatusCode = NO_CONTENT;
  } else
    setError(FORBIDDEN);
}

void HTTPResponse::_processDirectoryListing() {
  DIR *dir;
  struct dirent *entry;
  std::string path;

  if (!m_Location->autoindex)
    setError(FORBIDDEN);
  dir = opendir(m_ResourcePath.c_str());
  if (!dir)
    return setError(SERVER_ERROR);
  std::stringstream body;
  path = m_Request->getPath();
  if (path[path.length() - 1] != '/')
    path += "/";

  body << "<html><title>" << m_Request->getPath() << " directory listing"
       << "</title><body>";
  body << "<h1>" << path << " directory listing</h1><ul>";
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

void HTTPResponse::_processCgiBody() {
  char buff[READ_BUFFER_SIZE];
  ssize_t rbytes = m_Cgi->read(buff, READ_BUFFER_SIZE);

  if (rbytes > 0)
    HTTPParser::parseCgi(*this, buff, rbytes);

  if (rbytes < READ_BUFFER_SIZE && m_CgiDone) {
    std::stringstream errorBody;
    errorBody << this->m_StderrBuffer;
    appendBody(errorBody.str());
    m_Body.seal();
    m_PollState = SOCKET_WRITE;
    m_State = PROCESS_HEADERS;
  }
}

void HTTPResponse::_processBody() {
  _processResource();
  if (m_StatusCode == OK)
    _readFileToBody(m_ResourcePath);
  m_Body.seal();
  m_State = PROCESS_HEADERS;
}

void HTTPResponse::setError(statusCode status) {
  m_Body.clear();
  std::cout << "[RESPONSE] set error: " << status << std::endl;
  m_StatusCode = status;
  _processErrorBody();
  m_Body.seal();
  m_State = PROCESS_HEADERS;
  m_PollState = SOCKET_WRITE;
}

void HTTPResponse::_processResource() {
  if (m_Request->getMethod() != GET && m_Request->getMethod() != HEAD)
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
    std::vector<std::string> indexes = m_Location->indexes;

    for (std::vector<std::string>::const_iterator it = indexes.begin();
         it != indexes.end(); it++) {
      std::string index = *it;
      std::string indexPath = m_ResourcePath + index;
      resource = indexPath.c_str();
      if (resource.isFile() && resource.canRead()) {
        m_ResourcePath = indexPath;
        return;
      }
    }
    if (indexes.size() > 0) {
      setError(NOT_FOUND);
      return;
    }
    if (m_Location->autoindex)
      _processDirectoryListing();
    else
      setError(FORBIDDEN);
  } else
    setError(NOT_FOUND);
}

void HTTPResponse::_writeToCgi() {
  if (m_CgiDone) {
    m_State = PROCESS_HEADERS;
    m_PollState = SOCKET_WRITE;
    return;
  }
  HTTPBody &body = m_Request->getBody();
  m_Cgi->write(body);
  if (body.getSize() == 0 && m_Cgi->m_SocketBuffer.size() == 0)
    m_PollState = CGI_READ;
}

void HTTPResponse::_sendBody() {
  char buff[READ_BUFFER_SIZE];
  if (m_Request->getMethod() == HEAD ||
      (m_Body.getSize() == 0 && m_SocketBuffer.size() == 0)) {
    m_State = DONE;
    return;
  }
  size_t leftoverBytes = m_SocketBuffer.read(buff, READ_BUFFER_SIZE);
  size_t rbytes =
      m_Body.read(buff + leftoverBytes, READ_BUFFER_SIZE - leftoverBytes);

  size_t buffSize = leftoverBytes + rbytes;

  ssize_t wBytes = send(m_ClientFd, buff, buffSize, 0);
  if (wBytes <= 0) {
    std::cerr << "[RESPONSE ERROR]: could not send response to client."
              << std::endl;
    m_State = DONE;
    return;
  }
  if (static_cast<size_t>(wBytes) < buffSize)
    m_SocketBuffer.write(buff + wBytes, buffSize - wBytes);
  if (m_Body.getSize() <= 0 && m_SocketBuffer.size() <= 0)
    m_State = DONE;
}

bool HTTPResponse::resume(bool isCgiReady, bool isClientReady) {
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

  return m_State == DONE;
}

void HTTPResponse::cleanupCgiEnv() {
  env_ptrs.clear();
  cgi_env.clear();

  std::vector<char *> empty_ptrs;
  env_ptrs.swap(empty_ptrs);

  std::vector<std::string> empty_env;
  cgi_env.swap(empty_env);
}

// HTTPResponse::~HTTPResponse(void) {}
HTTPResponse::~HTTPResponse() {
  cleanupCgiEnv();
  if (m_Cgi) {
    m_Cgi->cleanup(false);
    delete m_Cgi;
    m_Cgi = NULL;
  }
  this->forceCleanup();
}
