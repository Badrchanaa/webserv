#ifndef _ConfigServer_
#define _ConfigServer_
#include "http.hpp"
#include <cstdlib>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Location.hpp"


struct ConfigServer {
  std::vector<int> ports;
  std::vector<std::string> server_names;
  std::string host;
  std::string body_size;
  int timeout;
  std::map<int, std::string> errors;
  std::vector<Location> locations;

  const Location *getLocation(const std::string &path) const;
  // const Location &getLocation(const std::string &path) const;
  ~ConfigServer();
};
#endif // !DEBUG