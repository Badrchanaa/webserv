#ifndef _ConfigStructs__
#define _ConfigStructs__
#include "http.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct MethodPair {
  const char *name;
  httpMethod method;
};

// enum httpMethod { METHOD_NONE = 0, GET = 1, POST = 2, DELETE = 4 };

struct Location {
  std::string uri;
  std::string root;
  bool autoindex;
  std::string upload;
  std::map<std::string, std::string> cgi;
  unsigned int allowed_methods;
  unsigned int allowed_cgi_methods;
  std::string index;

  bool isMethodAllowed(httpMethod method) const {
    return (allowed_methods & method) != 0;
  }

  std::string getIndexPath(const std::string &path) const {
    if (this->index.empty())
      return path;
    if (!path.empty() && path[path.size() - 1] == '/')
      return path + this->index;
    return path + "/" + this->index;
  }
};

struct ConfigServer {
  std::vector<int> ports;
  std::vector<std::string> server_names;
  std::string host;
  std::string body_size;
  // std::map<std::string, std::string> errors;  // Changed to int key
  std::map<int, std::string> errors;
  // Location location;  Befor
  std::vector<Location> locations; // After for Multi Locs

  const Location &getLocation(const std::string &path) const {
    const Location *bestMatch = NULL;
    size_t maxLength = 0;

  return locations[locations.size() - 1];
    for (std::vector<Location>::const_iterator it = locations.begin();
         it != locations.end(); ++it) {
      const Location &loc = *it;
      // i have some mistiks with this uri  don't forget about it
      if (path.find(loc.uri) == 0 && loc.uri.length() > maxLength) {
        maxLength = loc.uri.length();
        bestMatch = &loc;
      }
    }

    if (!bestMatch) {
      throw std::runtime_error("No matching location found");
    }
    return *bestMatch;
  }
  ~ConfigServer() {}
};

#endif // !DEBUG
