#include "../../Includes/Parsing.hpp"

bool ParseConfig::is_commented(std::string &line) {
  size_t i = 0;
  while (i < line.size() && line[i] == ' ')
    i++;
  return (line[i] == '#');
}

std::string ParseConfig::trim(const std::string &s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  size_t end = s.find_last_not_of(" \t\r\n");
  if (start == std::string::npos || end == std::string::npos)
    return "";
  return s.substr(start, end - start + 1);
}

void ParseConfig::split_key_value(const std::string &line, std::string &key,
                     std::string &value) {
  size_t colon = line.find(":");
  if (colon != std::string::npos) {
    key = this->trim(line.substr(0, colon));
    value = this->trim(line.substr(colon + 1));
  } else {
    key = this->trim(line);
    value = "";
  }
}


int ParseConfig::leading_spaces(const std::string &line) {
  int count = 0;
  for (size_t i = 0; i < line.size() && line[i] == ' '; ++i)
    count++;
  return count;
}

bool ParseConfig::is_list_item(const std::string &line) {
  std::string trimmed = this->trim(line);
  return trimmed.size() > 1 && trimmed[0] == '-' && trimmed[1] == ' ';
}

std::string ParseConfig::get_list_item(const std::string &line) {
  return this->trim(line.substr(2)); // after "- "
}

/* ++ Convert bitmask back to human-readable string ++ */
std::string ParseConfig::method_bit_to_string(unsigned int mask) {
  std::string result;
  for (size_t i = 0; i < sizeof(valid_methods) / sizeof(valid_methods[0]);
       ++i) {
    if (mask & valid_methods[i].method) {
      if (!result.empty())
        result += " ";
      result += valid_methods[i].name;
    }
  }
  return result.empty() ? "NONE" : result;
}

