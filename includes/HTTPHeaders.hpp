#ifndef __HTTPHEADERS_HPP__
#define __HTTPHEADERS_HPP__

#include "HTTPParseState.hpp"
#include <map>
#include <string>

class HTTPHeaders {
public:
  typedef std::map<std::string, std::string> header_map_t;

public:
  HTTPHeaders(void);
  virtual ~HTTPHeaders();

  virtual HTTPParseState &getParseState() = 0;
  virtual void onHeadersParsed() = 0;
  virtual void addHeader(std::string key, std::string value);
  virtual void addHeader(std::string key, size_t value);
  bool hasHeader(const char *key) const;
  bool removeHeader(const char *key);
  const header_map_t &getHeaders() const;
  const std::string &getHeader(const std::string &key) const;
  const std::string &getHeader(const char *key) const;

protected:
  header_map_t m_Headers;
};

#endif
