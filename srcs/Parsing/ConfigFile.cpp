#include "../../Includes/Parsing.hpp"

void Parsing::EnterServerContext() {
  if (inServer) {
    FinalizeServer();
  }
  inServer = true;
  context = "server";
  currentServer = ServerConfig();
}

void Parsing::ProcessPortValue(const std::string &value) {
  std::istringstream iss(value);
  std::string port_str;
  while (iss >> port_str) {
    if (port_str.find_first_not_of("0123456789") != std::string::npos) {
      std::cerr << "Invalid port number: " << port_str << std::endl;
      exit(EXIT_FAILURE);
    }
    currentServer.ports.push_back(std::atoi(port_str.c_str()));
  }
}

void Parsing::ProcessServerNameValue(const std::string &value) {
  std::istringstream iss(value);
  std::string name;
  while (iss >> name) {
    currentServer.server_names.push_back(name);
  }
}

void Parsing::ProcessServerKeyValue(const std::string &key,
                                    const std::string &value) {
  if (key == "port") {
    ProcessPortValue(value);
  } else if (key == "server_name") {
    ProcessServerNameValue(value);
  } else if (key == "host") {
    currentServer.host = value;
  } else if (key == "body_size") {
    currentServer.body_size = value;
  }
}

void Parsing::HandleIndentOne(const std::string &trimmed) {
  if (trimmed == "errors:") {
    context = "errors";
  } else if (trimmed == "location:") {
    context = "location";
  } else {
    std::string key, value;
    split_key_value(trimmed, key, value);
    if (SERVER_KEYS.find(key) == SERVER_KEYS.end()) {
      std::cerr << "Invalid server key: " << key << std::endl;
      exit(EXIT_FAILURE);
    }
    ProcessServerKeyValue(key, value);
  }
}

void Parsing::ProcessLocationKeyValue(const std::string &key,
                                      const std::string &value) {
  if (key == "uri") {
    currentServer.location.uri = value;
  } else if (key == "root") {
    currentServer.location.root = value;
  } else if (key == "autoindex") {
    currentServer.location.autoindex = (value == "on");
  } else if (key == "upload") {
    currentServer.location.upload = value;
  } else if (key == "cgi") {
    context = "cgi";
  } else if (key == "methods") {
    context = "methods";
  } else if (key == "methods_cgi") {
    context = "methods_cgi";
  }
}

void Parsing::HandleIndentThree(const std::string &trimmed) {
  if (context == "methods" || context == "methods_cgi" || context == "cgi") {
    context = "location";
  }

  if (context == "errors") {
    std::string key, value;
    split_key_value(trimmed, key, value);
    currentServer.errors[key] = value;
  } else if (context == "location") {
    std::string key, value;
    split_key_value(trimmed, key, value);
    ProcessLocationKeyValue(key, value);
  }
}

void Parsing::ProcessMethodContext(const std::string &trimmed) {
  std::string method = get_list_item(trimmed);
  HttpMethod bit = get_method_bit(method);
  if (bit == METHOD_NONE) {
    std::cerr << "Error: Invalid method '" << method << "'\n";
    exit(EXIT_FAILURE);
  }

  unsigned int &current_mask = (context == "methods")
                                   ? currentServer.location.allowed_methods
                                   : currentServer.location.allowed_cgi_methods;
  if (current_mask & bit) {
    std::cerr << "Error: Duplicate method '" << method << "'\n";
    exit(EXIT_FAILURE);
  }
  current_mask |= bit;
}

void Parsing::ProcessCgiContext(const std::string &trimmed) {
  std::string item = trimmed;
  if (is_list_item(item)) {
    item = get_list_item(item);
  }
  std::string key, value;
  split_key_value(item, key, value);
  currentServer.location.cgi[key] = value;
}

void Parsing::HandleIndentFive(const std::string &trimmed) {
  if (context == "methods" || context == "methods_cgi") {
    ProcessMethodContext(trimmed);
  } else if (context == "cgi") {
    ProcessCgiContext(trimmed);
  }
}

void Parsing::FinalizeServer() {
  if (!validate_server(currentServer)) {
    std::cerr << "Invalid server configuration" << std::endl;
    exit(EXIT_FAILURE);
  }
  this->AddServer(currentServer);
  inServer = false;
  context.clear();
}

void Parsing::ProcessLines(std::ifstream &infile) {
  std::string line;
  while (std::getline(infile, line)) {

    if (line.empty() || line[0] == '#')
      continue;

    int indent = leading_spaces(line);
    std::string trimmed = trim(line);
    if (trimmed.empty())
      continue;
    // std::cout << indent << " |line : " << line << std::endl;

    if (indent == 0 && trimmed == "server:") {
      EnterServerContext();
    } else if (inServer) {
      switch (indent) {
      case 1:
        HandleIndentOne(trimmed);
        break;
      case 3:
        HandleIndentThree(trimmed);
        break;
      case 5:
        HandleIndentFive(trimmed);
        break;
      default:
        std::cerr << "Unexpected indent level: " << indent << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  }
}

void Parsing::ResetParsingState() {
  inServer = false;
  context.clear();
  currentServer = ServerConfig();
}

void Parsing::ParseConfigFile(const char *FileName) {
  std::ifstream infile(FileName);
  if (!infile.is_open()) {
    std::cerr << "Failed to open config file: " << FileName << std::endl;
    exit(EXIT_FAILURE);
  }
  // this->context = "";
  // this->inServer = false;
  // this->currentServer

  ResetParsingState();
  ProcessLines(infile);

  if (inServer) {
    FinalizeServer();
  }
}

// void Parsing::ParseConfigFile(const char *FileName) {
//   std::ifstream infile(FileName);
//   std::string line, key, value;
//   ServerConfig currentServer;
//   std::string context = "";
//   bool inServer = false;
//
//   // std::cout << "i am here " << std::endl;
//   while (std::getline(infile, line)) {
//     // std::cout << "i am here " << std::endl;
//     if (line.empty() || line[0] == '#')
//       continue;
//     int indent = leading_spaces(line);
//     std::string trimmed = trim(line);
//     if (trimmed.empty())
//       continue;
//     std::cout << indent << " |line : " << line << std::endl;
//     if (indent == 0 && trimmed == "server:") {
//       inServer = true;
//       context = "server";
//     } else if (inServer && indent == 1) {
//       if (trimmed == "errors:") {
//         context = "errors";
//       } else if (trimmed == "location:") {
//         context = "location";
//       } else {
//         split_key_value(trimmed, key, value);
//         if (SERVER_KEYS.find(key) == SERVER_KEYS.end()) {
//           std::cerr << "Invalid server key: " << key << std::endl;
//           exit(EXIT_FAILURE);
//         }
//
//         // Handle multi-value fields
//         if (key == "port") {
//           std::cout << "value : " << value << " | " <<
//           std::atoi(value.c_str())
//                     << std::endl;
//           std::istringstream iss(value);
//           std::string method;
//           while (iss >> method) {
//             if (method.find_first_not_of("0123456789") != std::string::npos)
//             {
//               std::cerr << "Invalid server configuration" << std::endl;
//               exit(EXIT_FAILURE);
//             }
//             currentServer.ports.push_back(std::atoi(method.c_str()));
//           }
//         } else if (key == "server_name") {
//
//           std::istringstream iss(value);
//           std::string method;
//           while (iss >> method) {
//             currentServer.server_names.push_back(method);
//           }
//         } else if (key == "host")
//           currentServer.host = value;
//         else if (key == "body_size")
//           currentServer.body_size = value;
//       }
//     } else if (inServer && indent == 3) {
//       // Reset context to "location" if coming from sub-contexts
//       if (context == "methods" || context == "methods_cgi" ||
//           context == "cgi") {
//         context = "location";
//       }
//
//       if (context == "errors") {
//         split_key_value(trimmed, key, value);
//         // std::cout << "Errors : " << key << " | " << value << std::endl;
//         currentServer.errors[key] = value;
//       } else if (context == "location") {
//         split_key_value(trimmed, key, value);
//         if (key == "uri")
//           currentServer.location.uri = value;
//         else if (key == "root")
//           currentServer.location.root = value;
//         else if (key == "autoindex")
//           currentServer.location.autoindex = (value == "on");
//         else if (key == "upload")
//           currentServer.location.upload = value;
//         else if (key == "cgi") {
//           context = "cgi";
//         } else if (key == "methods") {
//           context = "methods";
//         } else if (key == "methods_cgi") {
//           context = "methods_cgi";
//         }
//       }
//     } else if (inServer && indent == 5) {
//
//       if (context == "methods" || context == "methods_cgi") {
//         std::string method = get_list_item(trimmed);
//         HttpMethod bit = get_method_bit(method);
//
//         if (bit == METHOD_NONE) {
//           std::cerr << "Error: Invalid method '" << method << "'\n";
//           exit(EXIT_FAILURE);
//         }
//
//         // Determine which methods mask to update
//         unsigned int &current_mask =
//             (context == "methods") ? currentServer.location.allowed_methods
//                                    :
//                                    currentServer.location.allowed_cgi_methods;
//
//         if (current_mask & bit) {
//           std::cerr << "Error: Duplicate method '" << method << "'\n";
//           exit(EXIT_FAILURE);
//         }
//         current_mask |= bit;
//
//       } else if (context == "cgi") {
//         std::string item = trimmed;
//         if (is_list_item(item)) {
//           item = get_list_item(item);
//         }
//         split_key_value(item, key, value);
//         currentServer.location.cgi[key] = value;
//       }
//     }
//   }
//   if (inServer) {
//     if (!validate_server(currentServer)) {
//       std::cerr << "Invalid server configuration" << std::endl;
//       exit(EXIT_FAILURE);
//     }
//     this->AddServer(currentServer);
//   }
// }

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
    // Check if key is wrapped in quotes
    if (pair.first.size() < 2 || pair.first.front() != '"' ||
        pair.first.back() != '"') {
      std::cerr << "Error code must be quoted: " << pair.first << std::endl;
      return false;
    }

    std::string code = pair.first.substr(1, pair.first.size() - 2);

    // Validate numeric code
    if (code.empty() ||
        code.find_first_not_of("0123456789") != std::string::npos) {
      std::cerr << "Invalid error code format: " << pair.first << std::endl;
      return false;
    }
    int status_code = std::atoi(code.c_str());
    if (status_code < 400 || status_code >= 600) {
      std::cerr << "Invalid HTTP error code: " << status_code << std::endl;
      return false;
    }
    // Validate path
    if (pair.second.empty() ||
        pair.second.find_first_of("\t\r\n ") != std::string::npos) {
      std::cerr << "Invalid error path: " << pair.second << std::endl;
      return false;
    }
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

bool validate_server(const ServerConfig &config) {
  for (int port : config.ports) {
    if (!validate_port(port)) {
      std::cout << "Port error" << std::endl;
      return false;
    }
  }

  if (!validate_body_size(config.body_size)) {
    std::cout << "body_siz error" << std::endl;
    return false;
  }

  if (!validate_error_paths(config.errors)) {
    std::cout << "errors error" << std::endl;
    return false;
  }

  return true;
}
