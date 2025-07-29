#ifndef __HTTP_HPP__
# define __HTTP_HPP__

# define CR '\r'
# define LF '\n'
# define SP ' '
# define HTAB '\t'
# define SLASH '/'
# define HTTP "HTTP"
# define MAX_CRLF_BYTES 100

typedef enum {
  METHOD_NONE = 0,
  GET = 1 << 0,
  POST = 1 << 1,
  DELETE = 1 << 2,
  PUT = 1 << 3
} httpMethod;

#include <string>
#include <map>
// #include "HTTPHeaders.hpp"
typedef std::map<std::string, std::string> header_map_t;

header_map_t	parseHeaderDirectives(std::string headerValue, std::string::size_type startPos);
std::string removeQuotes(std::string str);

#endif