#ifndef __Connection__
#define __Connection__

#include "CGIHandler.hpp"
#include "Config.hpp"
#include "ConfigStructs.hpp"
#include "FileDescriptor.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

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
  bool m_KeepAlive;
  HTTPResponse m_Response;
  connectionState m_State;

  /// Added by regex33
  Config &config;
  CGIHandler &Cgihandler;
  FileDescriptor client_fd;
  HTTPRequest m_Request;

  bool hasEvent;
  bool cgiEvent;
  bool socketEvent;
  uint32_t events;

  /* Reference to specific server config */
  ServerConfig &server_config; 
                               
  Connection(CGIHandler &cgihandler, ServerConfig &server, Config &conf, int f);
  // Connection(CGIHandler &cgihandler, Config &conf, ServerConfig &server ,int f);
  // int getCgiSocket() const;
  // Connection(CGIHandler &cgihandler, Config &conf, int f,
  //            struct sockaddr_storage a)
  // : client_fd(f), state(REQUEST_PARSING), addr(a) {}

  void resetEvents();
  void reset();

  bool keepAlive();

  void init_response();
  // struct sockaddr_storage addr; //what is this
};

#endif
