
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// bool validate_methods(unsigned int methods) {
//   const unsigned int ALL_VALID_METHODS = GET | POST | DELETE | PUT | HEAD;
//   return (methods & ~ALL_VALID_METHODS) == 0;
// }
//

const std::set<std::string> SERVER_KEYS = {"host",      "port",   "server_name",
                                           "body_size", "errors", "location"};
const std::set<std::string> LOCATION_KEYS = {
    "uri", "root", "methods", "methods_cgi", "autoindex", "upload", "cgi"};

enum HttpMethod {
  METHOD_NONE = 0,
  GET = 1 << 0,
  POST = 1 << 1,
  DELETE = 1 << 2
};

// Add these at the top with your enums
struct MethodPair {
  const char *name;
  HttpMethod method;
};

constexpr MethodPair valid_methods[] = {
    {"GET", GET}, {"POST", POST}, {"DELETE", DELETE}};

struct Location {
  std::string uri;
  std::string root;
  unsigned int allowed_methods = METHOD_NONE;     // Bitmask for regular methods
  unsigned int allowed_cgi_methods = METHOD_NONE; // Bitmask for CGI methods
  // std::vector<std::string> methods;
  // std::vector<std::string> methods_cgi;
  std::string autoindex;
  std::string upload;
  std::map<std::string, std::string> cgi;
};
struct ServerConfig {
  std::string host;
  // int port;
  // std::string server_name;
  std::vector<int> ports;
  std::vector<std::string> server_names;
  std::string body_size;
  std::map<std::string, std::string> errors;
  Location location;
};

HttpMethod get_method_bit(const std::string &method) {
  for (size_t i = 0; i < sizeof(valid_methods) / sizeof(valid_methods[0]);
       ++i) {
    if (method == valid_methods[i].name) {
      return valid_methods[i].method;
    }
  }
  return METHOD_NONE;
}

bool validate_port(int port) { return port > 0 && port <= 65535; }

bool validate_error_paths(const std::map<std::string, std::string> &errors) {
  for (const auto &pair : errors) {
    if (pair.first.empty() || pair.second.empty())
      return false;
    if (pair.first.find_first_not_of("0123456789") != std::string::npos)
      return false;
  }
  return true;
}
bool validate_body_size(const std::string &body_size) {
  if (body_size.empty())
    return false;
  size_t unit_pos = body_size.find_first_not_of("0123456789");
  if (unit_pos == std::string::npos)
    return true; // No unit is OK

  std::string unit = body_size.substr(unit_pos);
  return unit == "B" || unit == "K" || unit == "M" || unit == "G";
}

std::string trim(const std::string &s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  size_t end = s.find_last_not_of(" \t\r\n");
  if (start == std::string::npos || end == std::string::npos)
    return "";
  return s.substr(start, end - start + 1);
}

void split_key_value(const std::string &line, std::string &key,
                     std::string &value) {
  size_t colon = line.find(":");
  if (colon != std::string::npos) {
    key = trim(line.substr(0, colon));
    value = trim(line.substr(colon + 1));
  } else {
    key = trim(line);
    value = "";
  }
}

int leading_spaces(const std::string &line) {
  int count = 0;
  for (size_t i = 0; i < line.size() && line[i] == ' '; ++i)
    count++;
  return count;
}

bool is_list_item(const std::string &line) {
  std::string trimmed = trim(line);
  return trimmed.size() > 1 && trimmed[0] == '-' && trimmed[1] == ' ';
}

std::string get_list_item(const std::string &line) {
  return trim(line.substr(2)); // after "- "
}

// Convert bitmask back to human-readable string
std::string method_bit_to_string(unsigned int mask) {
  std::string result;
  for (const auto &m : valid_methods) {
    if (mask & m.method) {
      if (!result.empty())
        result += " ";
      result += m.name;
    }
  }
  return result.empty() ? "NONE" : result;
}

// Validation check before adding server
// bool validate_server(const ServerConfig& s) {
//     if (!validate_methods(s.location.allowed_methods)) {
//         std::cerr << "Invalid regular methods configuration\n";
//         return false;
//     }
//     if (!validate_methods(s.location.allowed_cgi_methods)) {
//         std::cerr << "Invalid CGI methods configuration\n";
//         return false;
//     }
//     return true;
// }

bool validate_server(const ServerConfig &config) {
  // Validate ports
  for (int port : config.ports) {
    if (!validate_port(port))
      return false;
  }

  // Validate body size
  if (!validate_body_size(config.body_size))
    return false;

  // Validate error pages
  if (!validate_error_paths(config.errors))
    return false;

  // Validate methods
  // if (!validate_methods(config.location.allowed_methods))
  //   return false;
  // if (!validate_methods(config.location.allowed_cgi_methods))
  //   return false;

  return true;
}

int main() {
  std::ifstream infile("web.yml");
  std::string line, key, value;
  std::vector<ServerConfig> servers;
  ServerConfig currentServer;
  std::string context = "";
  bool inServer = false;

  while (std::getline(infile, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    int indent = leading_spaces(line);
    std::string trimmed = trim(line);
    if (trimmed.empty())
      continue;
    std::cout << indent << " |line : " << line << std::endl;

    if (indent == 0 && trimmed == "server:") {
      // if (inServer) {
      //   servers.push_back(currentServer); // Save previous server
      //   currentServer = ServerConfig();   // Reset for new server
      // }
      inServer = true;
      context = "server";
    } else if (inServer && indent == 1) {
      if (trimmed == "errors:") {
        context = "errors";
      } else if (trimmed == "location:") {
        context = "location";
      } else {
        split_key_value(trimmed, key, value);
        if (SERVER_KEYS.find(key) == SERVER_KEYS.end()) {
          std::cerr << "Invalid server key: " << key << std::endl;
          exit(EXIT_FAILURE);
        }

        // Handle multi-value fields
        if (key == "port") {
          currentServer.ports.push_back(std::atoi(value.c_str()));
        } else if (key == "server_name") {
          currentServer.server_names.push_back(value);
        } else if (key == "host")
          currentServer.host = value;
        else if (key == "body_size")
          currentServer.body_size = value;
        // else if (key == "port")
        //   currentServer.port = std::atoi(value.c_str());
        // else if (key == "server_name")
        //   currentServer.server_name = value;
      }
    } else if (inServer && indent == 3) {
      // Reset context to "location" if coming from sub-contexts
      // if (context == "methods" || context == "methods_cgi" ||
      //     context == "cgi") {
      //   context = "location";
      // }

      if (context == "errors") {
        split_key_value(trimmed, key, value);
        // std::cout << "Errors : " << key << " | " << value << std::endl;
        currentServer.errors[key] = value;
      } else if (context == "location") {
        split_key_value(trimmed, key, value);
        // std::cout << "+++++++++++key : " << key << std::endl;
        if (key == "uri")
          currentServer.location.uri = value;
        else if (key == "root")
          currentServer.location.root = value;
        else if (key == "autoindex")
          currentServer.location.autoindex = value;
        else if (key == "upload")
          currentServer.location.upload = value;
        else if (key == "cgi") {
          context = "cgi";
        } else if (key == "methods") {
          context = "methods";
        } else if (key == "methods_cgi") { // Corrected typo
          context = "methods_cgi";
        }
      }
    } else if (inServer && indent == 5) {

      if (context == "methods" || context == "methods_cgi") {
        std::string method = get_list_item(trimmed);
        HttpMethod bit = get_method_bit(method);

        if (bit == METHOD_NONE) {
          std::cerr << "Error: Invalid method '" << method << "'\n";
          exit(EXIT_FAILURE);
        }

        // Determine which methods mask to update
        unsigned int &current_mask =
            (context == "methods") ? currentServer.location.allowed_methods
                                   : currentServer.location.allowed_cgi_methods;

        if (current_mask & bit) {
          std::cerr << "Error: Duplicate method '" << method << "'\n";
          exit(EXIT_FAILURE);
        }

        current_mask |= bit;

        // std::cout << "trimmed : " << trimmed << " | context : " << context
        // << std::endl;

        // if (context == "methods") {
        //   currentServer.location.methods.push_back(get_list_item(trimmed));
        // } else if (context == "methods_cgi") {
        //   currentServer.location.methods_cgi.push_back(get_list_item(trimmed));
      } else if (context == "cgi") {
        std::string item = trimmed;
        if (is_list_item(item)) {
          item = get_list_item(item);
        }
        split_key_value(item, key, value);
        currentServer.location.cgi[key] = value;
      }
      // else if (context == "cgi") {
      //   split_key_value(trimmed, key, value);
      //   currentServer.location.cgi[key] = value;
      // }
    }
  }

  // if (inServer)
  //   servers.push_back(currentServer); // Push last server config
  if (inServer) {
    if (!validate_server(currentServer)) {
      std::cerr << "Invalid server configuration" << std::endl;
      exit(EXIT_FAILURE);
    }
    servers.push_back(currentServer);
  }

  // 🎯 Print results
  std::cout << "servers : " << servers.size() << std::endl;
  for (size_t i = 0; i < servers.size(); ++i) {
    const ServerConfig &s = servers[i];
    std::cout << "==== Server #" << i + 1 << " ====\n";
    std::cout << "Host: " << s.host << "\n";
    // std::cout << "Port: " << s.port << "\n";
    std::cout << "server_names: " << std::endl;
    for (std::vector<std::string>::const_iterator it = s.server_names.begin();
         it != s.server_names.end(); it++) {
      std::cout << " - " << *it << std::endl;
    }

    std::cout << "Ports: " << std::endl;
    for (std::vector<int>::const_iterator it = s.ports.begin();
         it != s.ports.end(); it++) {
      std::cout << " - " << *it << std::endl;
    }
    // std::cout << "Server Name: " << s.server_name << "\n";
    std::cout << "Body Size: " << s.body_size << "\n";
    std::cout << "Errors:\n";
    for (std::map<std::string, std::string>::const_iterator it =
             s.errors.begin();
         it != s.errors.end(); ++it)
      std::cout << "  " << it->first << ": " << it->second << "\n";

    std::cout << "+++++++++++++++++++++++++++++++++++++++" << std::endl;

    std::cout << "Location:\n";
    std::cout << "  URI: " << s.location.uri << "\n";
    std::cout << "  Root: " << s.location.root << "\n";
    std::cout << "  Autoindex: " << s.location.autoindex << "\n";
    std::cout << "  Upload: " << s.location.upload << "\n";
    // std::cout << "  Methods: ";
    std::cout << "Methods: " << method_bit_to_string(s.location.allowed_methods)
              << "\n";
    // for (size_t j = 0; j < s.location.methods.size(); ++j)
    //   std::cout << s.location.methods[j] << " ";
    std::cout << std::endl;
    std::cout << "+++++++++++++++++++++++++++++++++++++++" << std::endl;
    std::cout << "\n  Errors:\n";
    for (std::map<std::string, std::string>::const_iterator it =
             s.errors.begin();
         it != s.errors.end(); ++it) {
      std::cout << "    " << it->first << ": " << it->second << "\n";
    }
    std::cout << "+++++++++++++++++++++++++++++++++++++++" << std::endl;
    std::cout << "CGI Methods: "
              << method_bit_to_string(s.location.allowed_cgi_methods) << "\n";
    // std::cout << " Methods CGI: " << std::endl;
    // for (size_t j = 0; j < s.location.methods_cgi.size(); ++j)
    //   std::cout << s.location.methods_cgi[j] << " ";
    std::cout << std::endl
              << "+++++++++++++++++++++++++++++++++++++++" << std::endl;
    std::cout << "\n  CGI:\n";
    for (std::map<std::string, std::string>::const_iterator it =
             s.location.cgi.begin();
         it != s.location.cgi.end(); ++it)
      std::cout << "    " << it->first << ": " << it->second << "\n";
    std::cout << std::endl;
  }

  return 0;
}

