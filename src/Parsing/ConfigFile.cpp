#include "Config.hpp"
#include <sys/wait.h>

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
}

void Config::ResetParsingState() {
  this->inServer = false;
  this->context.clear();
  this->currentServer = ConfigServer();
  this->currentLocation = Location();
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
    this->currentServer.host = value;
  } else if (key == "body_size") {
    this->currentServer.body_size = value;
  }
}

void Config::HandleIndentOne(const std::string &trimmed) {

  if (trimmed == "errors:") {
    this->FinalizeLocation();
    this->context = "errors";
  } else if (trimmed == "location:") {
    // std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
    // std::cout << "i am here location" << std::endl;
    // std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
    this->FinalizeLocation();
    this->context = "location";
    // this->currentLocation = Location();
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
    std::cout << "URI -> " << value << std::endl;
    this->currentLocation.uri = value;
  } else if (key == "root") {
    // std::cout << "********************************" << std::endl;
    // std::cout << "root :: " << value << std::endl;
    // std::cout << "********************************" << std::endl;
    this->currentLocation.root = value;
  } else if (key == "autoindex" && (value == "on" || value == "off")) {
    std::cout << "value : " << value << std::endl;
    this->currentLocation.autoindex = (value == "on");
  } else if (key == "upload") {
    this->currentLocation.upload = value;
  } else if (key == "cgi_uri") {
    this->currentLocation.cgi_uri = value;
  } else if (key == "cgi") {
    this->context = "cgi";
  } else if (key == "methods") {
    this->context = "methods";
  } else if (key == "methods_cgi") {
    this->context = "methods_cgi";
  } else if (key == "index") { // I Added index handling
    this->currentLocation.index = value;
  } else {
    std::cerr << "Unexpected location argument: " << value << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Config::ExitWithError(const std::string &message) {
  std::cerr << "Error: " << message << std::endl;
  exit(EXIT_FAILURE);
}
std::string Config::TrimQuotes(const std::string &str) {
  if (str.size() >= 2 && str[0] == '"' && str[str.size() - 1] == '"') {
    return str.substr(1, str.size() - 2);
  }
  return str;
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
  for (std::string::const_iterator it = key_str.begin(); it != key_str.end();
       ++it) {
    char c = *it;
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

  this->currentServer.errors[code] = value;
}

void Config::HandleIndentThree(const std::string &trimmed) {
  if (context == "methods" || context == "methods_cgi" || context == "cgi") {
    context = "location";
  }
  if (context == "errors") {
    this->FinalizeLocation();
    this->HandleErrorContext(trimmed);
  } else if (context == "location") {
    // std::cout << "helllo world->" << std::endl;
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
    // currentServer.location.cgi[key] = value; Old one
    this->currentLocation.cgi[key] = value;
  }
}

bool Config::isValidLocation() {
  // std::cout << "00000000000000000000000000000000000000000000" << std::endl;
  // if (this->currentLocation.uri.empty()) {
  //   std::cerr << "Missing 'uri' in location block" << std::endl;
  //   // return false;
  // }
  // if (this->currentLocation.root.empty()) {
  //   std::cerr << "Missing 'root' in location block" << std::endl;
  //   // return false;
  // }
  // if (this->currentLocation.upload.empty()) {
  //   std::cerr << "Missing 'upload' in location block" << std::endl;
  //   // return false;
  // }
  // if (this->currentLocation.allowed_methods == 0) {
  //   std::cerr << "No 'methods' specified in location block" << std::endl;
  // return false;
  // }
  // std::cout << "00000000000000000000000000000000000000000000" << std::endl;
  return !this->currentLocation.uri.empty() &&
         !this->currentLocation.root.empty() &&
         !this->currentLocation.upload.empty() &&
         this->currentLocation.allowed_methods;
}

void Config::FinalizeLocation() {
  // if (!this->isValidLocation()) {
  if (this->isValidLocation()) {
    currentServer.locations.push_back(currentLocation);
    std::cout << "==================================" << std::endl;
    std::cout << "currentLocation Size :: " << currentServer.locations.size()
              << std::endl;
    std::cout << "uri :: " << currentLocation.uri << std::endl;
    std::cout << "root :: " << currentLocation.root << std::endl;
    std::cout << "upload :: " << currentLocation.upload << std::endl;
    std::cout << "cgi_uri :: " << currentLocation.cgi_uri << std::endl;
    std::cout << "allowed_methods :: " << currentLocation.allowed_methods << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "cgi paths :: " << currentLocation.cgi[".py"] << "   |||||   " << currentLocation.cgi[".php"] << std::endl;
    std::cout << "==================================" << std::endl;
    currentLocation = Location(); // Reset
  }
}

void Config::FinalizeServer() {
  // Add any pending location
  // std::cout << "uri :: " << this->currentLocation.uri << std::endl;
  this->FinalizeLocation();

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
