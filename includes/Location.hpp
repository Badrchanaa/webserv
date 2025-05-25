#ifndef _Location_ 
#define _Location_
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


// enum httpMethod { METHOD_NONE = 0, GET = 1, POST = 2, DELETE = 4 };
#include <iostream>
#include <string>

struct Location {
  Location();


  std::string uri;
  std::string root;
  bool autoindex;
  std::string upload;
  std::string cgi_uri;
  std::vector<std::string> indexes;
  std::map<std::string, std::string> cgi;
  unsigned int allowed_methods;
  unsigned int allowed_cgi_methods;

  std::string getScriptPath(const std::string &scriptName) const;
  std::string getScriptName(const std::string &path) const;
  bool isCgiPath(const std::string &path) const;
  bool isMethodAllowed(httpMethod method) const;
  void parseValidIndexes(const std::string& indexLine);
  bool fileExists(const std::string& path) const;
  std::string getIndexPath(const std::string &path) const;
  bool isValidFormat(const std::string& filename);
};


#endif // !DEBUG
