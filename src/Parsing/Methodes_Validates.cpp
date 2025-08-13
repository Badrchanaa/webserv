#include "Config.hpp"


void Config::ProcessMethodContext(const std::string &trimmed) {
  std::string method = get_list_item(trimmed);
  httpMethod bit = get_method_bit(method);
  if (bit == METHOD_NONE)
    throw std::runtime_error("Error: Invalid method '" + method + "'\n");

  unsigned int &current_mask = (context == "methods")
                                   ? currentLocation.allowed_methods
                                   : currentLocation.allowed_cgi_methods;
  if (current_mask & bit)
    throw std::runtime_error("Error: Duplicate method '" + method + "'\n");
  current_mask |= bit;
}

httpMethod Config::get_method_bit(const std::string &method) {
  for (size_t i = 0; i < sizeof(valid_methods) / sizeof(valid_methods[0]);
       ++i) {
    if (method == valid_methods[i].name) {
      return valid_methods[i].method;
    }
  }
  return METHOD_NONE;
}


bool Config::validate_server(const ConfigServer &config) {
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
  return true;
}

bool Config::validate_port(int port) { return port > 0 && port <= 65535; }

bool Config::validate_timeout(const std::string &timeout) {
  if (timeout.empty())
    return false;
  size_t unit_pos = timeout.find_first_not_of("0123456789");
  if (unit_pos == std::string::npos)
    return true;
  return false;
}

bool Config::validate_body_size(const std::string &body_size) {
  if (body_size.empty())
    return false;
  size_t unit_pos = body_size.find_first_not_of("0123456789");
  if (unit_pos == std::string::npos)
    return true; // No unit is OK

  std::string unit = body_size.substr(unit_pos);
  return unit == "B" || unit == "K" || unit == "M" || unit == "G";
}
