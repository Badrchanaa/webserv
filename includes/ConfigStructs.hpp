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
#include <iostream>
#include <string>

struct Location {
  std::string uri;
  std::string root;
  bool autoindex;
  std::string upload;
  std::string cgi_uri;
  std::map<std::string, std::string> cgi;
  unsigned int allowed_methods;
  unsigned int allowed_cgi_methods;
  std::string index;

//  /app/cgi-bin/script.py
  std::string getScriptPath(const std::string &scriptName) const
  {
      std::string::size_type dotPos = scriptName.rfind('.');
      if (dotPos == std::string::npos )
        return (std::string(""));
    std::string ext;
      // if (dotPos == 0)
      //   ext = scriptName;
      // else 
      ext = scriptName.substr(dotPos);
      std::cout << "--------------------------------" << std::endl;
      std::cout << "ext " << ext << std::endl;
      std::cout << "--------------------------------" << std::endl;
      std::map<std::string, std::string>::const_iterator it = cgi.find(ext);
      if (it == cgi.end())
        return (std::string(""));
        // return NULL;
      return it->second;
  }

  std::string getScriptName(const std::string &path) const
  {
    std::string::size_type slashPos = path.rfind('/');
    if (slashPos == std::string::npos || path.length() == 1)
       return (std::string(""));
    std::cout << "++++++++++++++++++++++++++++ -> " <<  slashPos << std::endl;
    
    std::string temp = path.substr(slashPos + 1);
    std::cout << "++++++++++++++++++++++++++++ " << std::endl;
    return temp;
  }

  bool isCgiPath(const std::string &path) const
  {
    std::string filename = getScriptName(path);
    std::cout << "fileName : " << filename  << std::endl;
    std::string scriptPath = getScriptPath(filename);
    std::cout << "scriptPath: " << scriptPath  << std::endl;
    
    return (!scriptPath.empty() &&  (this->uri + this->cgi_uri == path.substr(0, path.length() - filename.length())));
  }

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
  Location() : autoindex(0), allowed_methods(0), allowed_cgi_methods(0) {}
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
      // return loc;
      // i have some mistiks with this uri  don't forget about it
      if (path.find(loc.uri) == 0 && loc.uri.length() > maxLength) {
        maxLength = loc.uri.length();
        bestMatch = &loc;
      }
    }

    if (!bestMatch) {
      std::cout << "location uri form getLocation function :: "
                << locations[locations.size() - 1].uri << std::endl;
      return locations[locations.size() - 1];
      // throw std::runtime_error("No matching location found");
    }
    return *bestMatch;
  }
  ~ConfigServer() {}
};

#endif // !DEBUG
