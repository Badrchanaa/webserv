#ifndef __Connection__
#define __Connection__

#include "CGIHandler.hpp"
#include "Config.hpp"
#include "Location.hpp"
#include "FileDescriptor.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
// #include "WebServer.hpp"
#include "EpollManager.hpp"
#include <ctime>

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
  bool cgi_Added;
  bool client_Added;
  bool m_KeepAlive;
  HTTPResponse m_Response;
  connectionState m_State;
  std::vector<ConfigServer> &servers;

/// Added by regex33
  // Config &config;
  // CGIHandler &Cgihander;
  // FileDescriptor client_fd;
  int client_fd;

  HTTPRequest m_Request;

  bool hasEvent;
  bool cgiEvent;
  bool socketEvent;



  uint32_t events;

  // Time out
  // std::time_t cgi_last_activity;
  // std::time_t client_last_activity;
  std::time_t last_activity;
  /* Reference to specific server config */
  // ConfigServer &config_server; 
                               
  // Connection(CGIHandler &cgihandler, std::vector<ConfigServer> &server, Config &conf, int f);
  Connection(std::vector<ConfigServer> &servers, int f);
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
