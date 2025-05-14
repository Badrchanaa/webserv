#include "Config.hpp"

const ConfigServer &Config::getServerByName(std::vector<ConfigServer> &servers,
                                            std::string name) {
  for (size_t i = 0; i < servers.size(); ++i) {
    const ConfigServer &server = servers[i];
    for (size_t j = 0; j < server.server_names.size(); ++j) {
      if (server.server_names[j] == name) {
        return server;
      }
    }
  }

  if (!servers.empty()) {
    return servers[0];
  }
  return servers[0];

  // std::cerr << "No servers are configured." << std::endl;
  // exit(EXIT_FAILURE);
}

void Config::ResetParsingState() {
  inServer = false;
  context.clear();
  currentServer = ConfigServer();
}

void Config::EnterServerContext() {
  if (inServer) {
    this->FinalizeServer();
  }
  inServer = true;
  context = "server";
  currentServer = ConfigServer();
}

void Config::ProcessPortValue(const std::string &value) {
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

void Config::ProcessServerNameValue(const std::string &value) {
  std::istringstream iss(value);
  std::string name;
  while (iss >> name) {
    currentServer.server_names.push_back(name);
  }
}

void Config::ProcessServerKeyValue(const std::string &key,
                                   const std::string &value) {
  if (key == "port") {
    this->ProcessPortValue(value);
  } else if (key == "server_name") {
    this->ProcessServerNameValue(value);
  } else if (key == "host") {
    currentServer.host = value;
  } else if (key == "body_size") {
    currentServer.body_size = value;
  }
}

void Config::HandleIndentOne(const std::string &trimmed) {
  if (trimmed == "errors:") {
    context = "errors";
  } else if (trimmed == "location:") {
    context = "location";
  } else {
    std::string key, value;
    this->split_key_value(trimmed, key, value);
    if (SERVER_KEYS.find(key) == SERVER_KEYS.end()) {
      std::cerr << "Invalid server key: " << key << std::endl;
      exit(EXIT_FAILURE);
    }
    this->ProcessServerKeyValue(key, value);
  }
}

void Config::ProcessLocationKeyValue(const std::string &key,
                                     const std::string &value) {
  if (key == "uri") {
    currentServer.location.uri = value;
  } else if (key == "root") {
    currentServer.location.root = value;
  } else if (key == "autoindex" && (value == "on" || value == "off")) {
    std::cout << "value : " << value << std::endl;
    currentServer.location.autoindex = (value == "on");
  } else if (key == "upload") {
    currentServer.location.upload = value;
  } else if (key == "cgi") {
    context = "cgi";
  } else if (key == "methods") {
    context = "methods";
  } else if (key == "methods_cgi") {
    context = "methods_cgi";
  } else if (key == "index") { // I Added index handling
    currentLocation.index = value;
  } else {
    std::cerr << "Unexpected location argument: " << value << std::endl;
    exit(EXIT_FAILURE);
  }
}

// void Config::HandleIndentThree(const std::string &trimmed) {
//   if (context == "methods" || context == "methods_cgi" || context == "cgi") {
//     context = "location";
//   }
//
//   if (context == "errors") {
//     std::string key_str, value;
//     this->split_key_value(trimmed, key_str, value);
//
//     if (!key_str.empty()) {
//       if (key_str[0] == '"' && key_str[key_str.size() - 1] == '"') {
//         key_str = key_str.substr(1, key_str.size() - 2);
//       }
//     }
//     if (key_str.empty() ||
//         key_str.find_first_not_of("0123456789") != std::string::npos) {
//       std::cerr << "Error: Invalid error code format '" << key_str
//                 << "'. Must contain only digits." << std::endl;
//       exit(EXIT_FAILURE);
//     }
//
//     int code = 0;
//     for (size_t i = 0; i < key_str.size(); ++i) {
//       int digit = key_str[i] - '0';
//       if (code >= 600) {
//         std::cerr << "Error: Invalid HTTP error code " << code
//                   << ". Must be between 400-599." << std::endl;
//         exit(EXIT_FAILURE);
//       }
//       code = code * 10 + digit;
//     }
//     if (code < 400 || code >= 600) {
//       std::cerr << "Error: Invalid HTTP error code " << code
//                 << ". Must be between 400-599." << std::endl;
//       exit(EXIT_FAILURE);
//     }
//
//     if (value.empty()) {
//       std::cerr << "Error: Empty error page path for code " << code
//                 << std::endl;
//       exit(EXIT_FAILURE);
//     }
//     if (value.find_first_of(" \t\r\n") != std::string::npos) {
//       std::cerr << "Error: Error page path contains whitespace for code "
//                 << code << std::endl;
//       exit(EXIT_FAILURE);
//     }
//     currentServer.errors[code] = value;
//   } else if (context == "location") {
//     std::string key, value;
//     this->split_key_value(trimmed, key, value);
//
//     if (LOCATION_KEYS.find(key) == LOCATION_KEYS.end()) {
//       std::cerr << "Error: Unknown location directive '" << key << "'"
//                 << std::endl;
//       exit(EXIT_FAILURE);
//     }
//     this->ProcessLocationKeyValue(key, value);
//   } else {
//     std::cerr << "Error: Unexpected context '" << context
//               << "' at indent level 3" << std::endl;
//     exit(EXIT_FAILURE);
//   }
// }

void Config::ExitWithError(const std::string &message) {
  std::cerr << "Error: " << message << std::endl;
  exit(EXIT_FAILURE);
}
std::string Config::TrimQuotes(const std::string &str) {
  return (str.size() >= 2 && str.front() == '"' && str.back() == '"')
             ? str.substr(1, str.size() - 2)
             : str;
}

void Config::ValidateErrorCodeFormat(const std::string &key_str) {
  if (key_str.empty() ||
      key_str.find_first_not_of("0123456789") != std::string::npos) {
    this->ExitWithError("Invalid error code format '" + key_str +
                        "'. Must contain only digits.");
  }
}

int Config::ConvertToErrorCode(const std::string &key_str) {
  int code = 0;
  for (char c : key_str) {
    code = code * 10 + (c - '0');
    if (code >= 600)
      this->ExitWithError("Invalid HTTP error code " + Config::to_String(code) +
                          ". Must be 400-599.");
  }
  return code;
}

void Config::ValidateErrorPath(const std::string &path, int code) {
  if (path.empty())
    this->ExitWithError("Empty error page path for code " +
                        Config::to_String(code));
  if (path.find_first_of(" \t\r\n") != std::string::npos) {
    this->ExitWithError("Error page path contains whitespace for code " +
                        Config::to_String(code));
  }
}

void Config::HandleErrorContext(const std::string &trimmed) {
  std::string key_str, value;
  this->split_key_value(trimmed, key_str, value);

  key_str = this->TrimQuotes(key_str);
  this->ValidateErrorCodeFormat(key_str);

  const int code = this->ConvertToErrorCode(key_str);
  if (code < 400 || code >= 600) {
    this->ExitWithError("Invalid HTTP error code " + Config::to_String(code) +
                        ". Must be 400-599.");
  }
  this->ValidateErrorPath(value, code);

  currentServer.errors[code] = value;
}

void Config::HandleIndentThree(const std::string &trimmed) {
  if (context == "methods" || context == "methods_cgi" || context == "cgi") {
    context = "location";
  }
  if (context == "errors") {
    this->HandleErrorContext(trimmed);
  } else if (context == "location") {
    this->HandleLocationContext(trimmed);
  } else {
    this->ExitWithError("Unexpected context '" + context +
                        "' at indent level 3");
  }
}

// Location context handling
void Config::HandleLocationContext(const std::string &trimmed) {
  std::string key, value;
  this->split_key_value(trimmed, key, value);

  if (LOCATION_KEYS.find(key) == LOCATION_KEYS.end()) {
    this->ExitWithError("Unknown location directive '" + key + "'");
  }
  this->ProcessLocationKeyValue(key, value);
}

void Config::HandleIndentFive(const std::string &trimmed) {
  if (context == "methods" || context == "methods_cgi") {
    this->ProcessMethodContext(trimmed);
  } else if (context == "cgi") {
    std::string item = trimmed;
    if (is_list_item(item)) {
      item = this->get_list_item(item);
    }
    std::string key, value;
    this->split_key_value(item, key, value);
    currentServer.location.cgi[key] = value;
  }
}

void Config::FinalizeServer() {
  // Add any pending location
  if (context == "location") {
    currentServer.locations.push_back(currentLocation);
    context.clear();
  }

  if (!validate_server(currentServer)) {
    std::cerr << "Invalid server configuration" << std::endl;
    exit(EXIT_FAILURE);
  }
  this->AddServer(currentServer);
  inServer = false;
  context.clear();
}

void Config::ProcessLines(std::ifstream &infile) {
  std::string line;
  while (std::getline(infile, line)) {

    // std::cout << "Lline : " << line << "| char : " << line[0] << std::endl;
    if (line.empty() || this->is_commented(line))
      continue;

    int indent = this->leading_spaces(line);
    std::string trimmed = trim(line);
    if (trimmed.empty())
      continue;
    // std::cout << indent << " |line : " << line << std::endl;

    if (indent == 0 && trimmed == "server:") {
      this->EnterServerContext();
    } else if (inServer) {
      switch (indent) {
      case 1:
        this->HandleIndentOne(trimmed);
        break;
      case 3:
        this->HandleIndentThree(trimmed);
        break;
      case 5:
        this->HandleIndentFive(trimmed);
        break;
      default:
        std::cerr << "Unexpected indent level: " << indent << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  }
}

void Config::ParseConfigFile(const char *FileName) {
  std::ifstream infile(FileName);
  if (!infile.is_open()) {
    std::cerr << "Failed to open config file: " << FileName << std::endl;
    exit(EXIT_FAILURE);
  }

  this->ResetParsingState();
  this->ProcessLines(infile);

  if (inServer) {
    this->FinalizeServer();
  }
}
