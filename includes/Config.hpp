#ifndef _Config__
#define _Config__
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "ConfigStructs.hpp"

#define DEFAULT_PATH "./Config/default.yml"

const std::string server_keys_arr[] = {"host",      "port",   "server_name",
                                       "body_size", "errors", "location"};
const std::set<std::string>
    SERVER_KEYS(server_keys_arr, server_keys_arr + sizeof(server_keys_arr) /
                                                       sizeof(std::string));

const std::string location_keys_array[] = {
    "uri", "root", "methods", "methods_cgi", "autoindex", "upload", "cgi"};
const std::set<std::string> LOCATION_KEYS(location_keys_array,
                                          location_keys_array +
                                              sizeof(location_keys_array) /
                                                  sizeof(std::string));

const MethodPair valid_methods[] = {
    {"GET", GET}, {"POST", POST}, {"DELETE", DELETE}};

class Config {

private:
  std::vector<ConfigServer> servers;
  ConfigServer currentServer;
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
  httpMethod get_method_bit(const std::string &method);
  bool validate_port(int port);
  bool validate_server(const ConfigServer &config);

  void ExitWithError(const std::string &message);
  void ValidateErrorCodeFormat(const std::string &key_str);
  int ConvertToErrorCode(const std::string &key_str);
  void ValidateErrorPath(const std::string &path, int code);
  void HandleErrorContext(const std::string &trimmed);

public:
  // My Templet HHHHHHHHHHHHHHHHHH --> Debuger Don't Remove the template
  template <typename T> std::string to_String(const T &value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }

  Config() {}
  ~Config() {}
  Config(const Config &other) { *this = other; }
  Config &operator=(const Config &other) {
    (void)other;
    return *this;
  }
  /// Geters
  int ServersNumber() { return this->servers.size(); }
  ConfigServer &getServer(int index) { return this->servers[index]; }

  // returns serverconfig based on name parameter
  static const ConfigServer &getServerByName(std::vector<ConfigServer> &servers,
                                             std::string name);

  /// Seters
  void AddServer(ConfigServer &ref) { this->servers.push_back(ref); }
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
