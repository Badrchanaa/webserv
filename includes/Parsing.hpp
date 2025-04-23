#ifndef _Parsing_
#define _Parsing_
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// typedef
#define DEFAULT_PATH "./Config/default.yml"
// const std::set<std::string> SERVER_KEYS = {"host",      "port",
// "server_name",
//                                            "body_size", "errors",
//                                            "location"};
const std::string server_keys_arr[] = {"host",      "port",   "server_name",
                                       "body_size", "errors", "location"};
const std::set<std::string>
    SERVER_KEYS(server_keys_arr, server_keys_arr + sizeof(server_keys_arr) /
                                                       sizeof(std::string));
// const std::set<std::string> LOCATION_KEYS = {
//     "uri", "root", "methods", "methods_cgi", "autoindex", "upload", "cgi"};

const std::string location_keys_array[] = {
    "uri", "root", "methods", "methods_cgi", "autoindex", "upload", "cgi"};
const std::set<std::string> LOCATION_KEYS(location_keys_array,
                                          location_keys_array +
                                              sizeof(location_keys_array) /
                                                  sizeof(std::string));

enum HttpMethod {
  METHOD_NONE = 0,
  GET = 1 << 0,
  POST = 1 << 1,
  DELETE = 1 << 2
};

struct MethodPair {
  const char *name;
  HttpMethod method;
};

// constexpr MethodPair valid_methods[] = {
//     {"GET", GET}, {"POST", POST}, {"DELETE", DELETE}};

const MethodPair valid_methods[] = {
    {"GET", GET}, {"POST", POST}, {"DELETE", DELETE}};
struct Location {
  std::string uri;
  std::string root;
  // unsigned int allowed_methods = METHOD_NONE;     // Bitmask for regular // this not work in cpp98 so i use constracter
  // methods unsigned int allowed_cgi_methods = METHOD_NONE; // Bitmask for CGI
  // methods
  unsigned int allowed_methods;
  unsigned int allowed_cgi_methods;
  bool autoindex;
  std::string upload;
  std::map<std::string, std::string> cgi;
  Location() : allowed_methods(METHOD_NONE), allowed_cgi_methods(METHOD_NONE) {}
};
struct ServerConfig {
  std::string host;
  std::vector<int> ports;
  std::vector<std::string> server_names;
  std::string body_size;
  std::map<std::string, std::string> errors;
  Location location;
};

class ParseConfig {
private:
  std::vector<ServerConfig> servers;

  ServerConfig currentServer;
  std::string context;
  bool inServer;

  // # ===============================
  void ResetParsingState();
  void ProcessLines(std::ifstream &infile);
  void ProcessLine(const std::string &line);
  void EnterServerContext();
  void HandleIndentOne(const std::string &trimmed);
  void HandleIndentThree(const std::string &trimmed);
  void HandleIndentFive(const std::string &trimmed);
  void ProcessServerKeyValue(const std::string &key, const std::string &value);
  void ProcessPortValue(const std::string &value);
  void ProcessServerNameValue(const std::string &value);
  void ProcessLocationKeyValue(const std::string &key,
                               const std::string &value);
  void ProcessMethodContext(const std::string &trimmed);
  void FinalizeServer();
  bool is_commented(std::string &line);
  bool validate_body_size(const std::string &body_size);
  bool validate_error_paths(const std::map<std::string, std::string> &errors);
  HttpMethod get_method_bit(const std::string &method);
  bool validate_port(int port);
  bool validate_server(const ServerConfig &config);



public:
  ParseConfig() {}
  ~ParseConfig() {}
  ParseConfig(const ParseConfig &other) { *this = other; }
  ParseConfig &operator=(const ParseConfig &other) {
    (void)other;
    return *this;
  }
  /// Geters
  int ServersNumber() { return this->servers.size(); }
  ServerConfig getServer(int index) { return this->servers[index]; }
  /// Seters
  void AddServer(ServerConfig &ref) { this->servers.push_back(ref); }
  /// Main
  void ParseConfigFile(const char *FileName);

  // +++++++++++ Helps Functions +++++++++++//
  std::string trim(const std::string &s);
  void split_key_value(const std::string &line, std::string &key,
                       std::string &value);
  int leading_spaces(const std::string &line);
  bool is_list_item(const std::string &line);
  std::string get_list_item(const std::string &line);
  std::string method_bit_to_string(unsigned int mask);
};


#endif // !DEBUG
