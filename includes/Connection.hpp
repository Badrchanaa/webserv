#ifndef __Connection__
#define __Connection__

#include "CGIHandler.hpp"
#include "Config.hpp"
#include "ConfigStructs.hpp"
#include "FileDescriptor.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "WebServer.hpp"

// CGI LINK: https://datatracker.ietf.org/doc/html/rfc3875
class Connection {

public:
  typedef enum {
    REQUEST_PARSING,
    RESPONSE_PROCESSING,
  } connectionState;

  // private:

public:
  /// Added by bchanaa
  bool cgiFdAdded;
  bool socketFdAdded;
  bool m_KeepAlive;
  HTTPResponse m_Response;
  connectionState m_State;

  /// Added by regex33
  // Config &config;
  // CGIHandler &Cgihander;
  // FileDescriptor client_fd;
  int client_fd;

  HTTPRequest m_Request;

  bool hasEvent;
  bool cgiEvent;
  bool socketEvent;

  bool cgi_Added;
  bool client_Added;


  uint32_t events;

  /* Reference to specific server config */
  // ConfigServer &config_server; 
                               
  // Connection(CGIHandler &cgihandler, std::vector<ConfigServer> &server, Config &conf, int f);
  Connection(std::vector<ConfigServer> &server, int f);
  ~Connection()
  {
  } 
  // Connection(CGIHandler &cgihandler, Config &conf, ServerConfig &server ,int f);
  // int getCgiSocket() const;
  // Connection(CGIHandler &cgihandler, Config &conf, int f,
  //            struct sockaddr_storage a)
  // : client_fd(f), state(REQUEST_PARSING), addr(a) {}

  void resetEvents();
  void reset();

  bool keepAlive();

  void init_response(EpollManager& epollManager, CGIHandler& cgiHandler);
  // struct sockaddr_storage addr; //what is this
};

#endif
