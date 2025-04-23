#include "Parsing.hpp"

void ParseConfig::ProcessMethodContext(const std::string &trimmed) {
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

HttpMethod ParseConfig::get_method_bit(const std::string &method) {
  for (size_t i = 0; i < sizeof(valid_methods) / sizeof(valid_methods[0]);
       ++i) {
    if (method == valid_methods[i].name) {
      std::cout << "method : " << valid_methods[i].name << "|"
                << valid_methods[i].method << std::endl;
      return valid_methods[i].method;
    }
  }
  return METHOD_NONE;
}

bool ParseConfig::validate_error_paths(
    const std::map<std::string, std::string> &errors) {
  for (std::map<std::string, std::string>::const_iterator it = errors.begin();
       it != errors.end(); ++it) {
    const std::string &key = it->first;
    const std::string &value = it->second;

    if (key.size() < 2 || key[0] != '"' || key[key.size() - 1] != '"') {
      std::cerr << "Error code must be quoted: " << key << std::endl;
      return false;
    }

    std::string code = key.substr(1, key.size() - 2);

    if (code.empty() ||
        code.find_first_not_of("0123456789") != std::string::npos) {
      std::cerr << "Invalid error code format: " << key << std::endl;
      return false;
    }

    int status_code = std::atoi(code.c_str());
    if (status_code < 400 || status_code >= 600) {
      std::cerr << "Invalid HTTP error code: " << status_code << std::endl;
      return false;
    }

    if (value.empty() || value.find_first_of("\t\r\n ") != std::string::npos) {
      std::cerr << "Invalid error path: " << value << std::endl;
      return false;
    }
  }
  return true;
}

bool ParseConfig::validate_server(const ServerConfig &config) {
  for (std::vector<int>::const_iterator it = config.ports.begin();
       it != config.ports.end(); it++) {
    if (!validate_port(*it)) {
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

bool ParseConfig::validate_port(int port) { return port > 0 && port <= 65535; }

bool ParseConfig::validate_body_size(const std::string &body_size) {
  if (body_size.empty())
    return false;
  size_t unit_pos = body_size.find_first_not_of("0123456789");
  if (unit_pos == std::string::npos)
    return true; // No unit is OK

  std::string unit = body_size.substr(unit_pos);
  return unit == "B" || unit == "K" || unit == "M" || unit == "G";
}
