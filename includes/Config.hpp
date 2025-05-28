#ifndef _Config__
#define _Config__
#include <climits>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "http.hpp"

#include "Location.hpp"
#include "ConfigServer.hpp"
#include "Constants.hpp"


class Config {

private:
  std::vector<ConfigServer> servers;
  ConfigServer currentServer;
  Location currentLocation;
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
  void FinalizeLocation();
  bool is_commented(std::string &line);
  bool validate_body_size(const std::string &body_size);
  bool validate_error_paths(const std::map<std::string, std::string> &errors);
  httpMethod get_method_bit(const std::string &method);
  bool validate_port(int port);
  bool validate_server(const ConfigServer &config);

  void ValidateErrorCodeFormat(const std::string &key_str);
  int ConvertToErrorCode(const std::string &key_str);
  void ValidateErrorPath(const std::string &path, int code);
  void HandleErrorContext(const std::string &trimmed);
  std::string TrimQuotes(const std::string &str);
  void HandleLocationContext(const std::string &trimmed);

  bool isValidLocation();
public:
  // My Templet HHHHHHHHHHHHHHHHHH --> Debuger Don't Remove the template
  template <typename T> std::string to_String(const T &value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }

  Config();
  ~Config() ;
  Config(const Config &other) ;
  Config &operator=(const Config &other) ;
  /// Geters
  int ServersNumber();
  ConfigServer &getServer(int index);

  // returns serverconfig based on name parameter
  static const ConfigServer &getServerByName(std::vector<ConfigServer> &servers,
                                             std::string name);

  /// Seters
  void AddServer(ConfigServer &ref);
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

  bool validate_timeout(const std::string &timeout);
  int safe_atoi(const char* str);
  // void ProcessIndexValue(const std::string &value);
  void creatDefaultServer();
  void printServersWithLocations() const;
  std::vector<ConfigServer> &getServers() {
    return servers;
  }

};

#endif // !DEBUG
