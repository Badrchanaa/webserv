#include "Parsing.hpp"

void ParseConfig::ResetParsingState() {
  inServer = false;
  context.clear();
  currentServer = ServerConfig();
}

void ParseConfig::EnterServerContext() {
  if (inServer) {
    this->FinalizeServer();
  }
  inServer = true;
  context = "server";
  currentServer = ServerConfig();
}

void ParseConfig::ProcessPortValue(const std::string &value) {
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

void ParseConfig::ProcessServerNameValue(const std::string &value) {
  std::istringstream iss(value);
  std::string name;
  while (iss >> name) {
    currentServer.server_names.push_back(name);
  }
}

void ParseConfig::ProcessServerKeyValue(const std::string &key,
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

void ParseConfig::HandleIndentOne(const std::string &trimmed) {
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

void ParseConfig::ProcessLocationKeyValue(const std::string &key,
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
  } else {
    std::cerr << "Unexpected location argument: " << value << std::endl;
    exit(EXIT_FAILURE);
  }
}

void ParseConfig::HandleIndentThree(const std::string &trimmed) {
  if (context == "methods" || context == "methods_cgi" || context == "cgi") {
    context = "location";
  }

  if (context == "errors") {
    std::string key, value;
    this->split_key_value(trimmed, key, value);
    currentServer.errors[key] = value;
  } else if (context == "location") {
    std::string key, value;
    this->split_key_value(trimmed, key, value);
    this->ProcessLocationKeyValue(key, value);
  }
}

void ParseConfig::HandleIndentFive(const std::string &trimmed) {
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

void ParseConfig::FinalizeServer() {
  if (!this->validate_server(currentServer)) {
    std::cerr << "Invalid server configuration" << std::endl;
    exit(EXIT_FAILURE);
  }
  this->AddServer(currentServer);
  inServer = false;
  context.clear();
}

void ParseConfig::ProcessLines(std::ifstream &infile) {
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

void ParseConfig::ParseConfigFile(const char *FileName) {
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
