#ifndef _Constants_
#define _Constants_
#include <climits>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "http.hpp"

// #include "Location.hpp"
// #include "ConfigServer.hpp"

#define DEFAULT_PATH "./Config/default.yml"

struct MethodPair {
  const char *name;
  httpMethod method;
};

const std::string server_keys_arr[] = {"host", "timeout",  "port",   "server_name", "body_size", "errors", "location"};
const std::set<std::string> SERVER_KEYS(server_keys_arr, server_keys_arr + sizeof(server_keys_arr) / sizeof(std::string));

const std::string location_keys_array[] = { "index", "uri", "root", "methods", "methods_cgi", "autoindex", "upload", "cgi", "cgi_uri"};

const std::set<std::string> LOCATION_KEYS(location_keys_array, location_keys_array + sizeof(location_keys_array) / sizeof(std::string));

const std::string location_required_params[] = { "index", "uri", "root", "methods", "autoindex", "upload"};

const MethodPair valid_methods[] = {{"GET", GET}, {"POST", POST}, {"DELETE", DELETE}};


#endif