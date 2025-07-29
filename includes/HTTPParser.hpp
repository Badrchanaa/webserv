#ifndef __HTTPPARSER_HPP__
# define __HTTPPARSER_HPP__

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "HTTPParseState.hpp"
#include <string>
#include <stdint.h>
#include "http.hpp"

/// @brief a static class for parsing HTTP requests.
class HTTPParser
{
	public:
		/// @brief begins/resumes http request parsing.
		/// @param request HTTP Request class
		/// @param buff content to parse
		/// @param len content length
		static void	parseRequest(HTTPRequest &request, const char *buff, size_t len);
		static void	parseCgi(HTTPResponse &response, const char *buff, size_t len);

		// static HTTPHeaders::header_map_t	parseHeaderDirectives(std::string headerValue, std::string::size_type startPos = 0);

		static const uint8_t TOKEN_ALLOWED_CHARS[128];
		
	private:
		HTTPParser(void);
		//unsigned int	_parseRequestLine(HTTPRequest &request, char *buff, size_t len) const;
		static size_t	_parseStart(HTTPRequest &request, const char *buff, size_t start, size_t len);
		static size_t	_parseMethod(HTTPRequest &request, const char *buff, size_t start, size_t len);
		static size_t	_parseTarget(HTTPRequest &request, const char *buff, size_t start, size_t len);
		static size_t	_parseVersion(HTTPRequest &request, const char *buff, size_t start, size_t len);
		static size_t	_parseVersionNumber(HTTPRequest &request, const char *buff, size_t start, size_t len);

		// static size_t	_skipHeaderField(const char *buff, size_t start, size_t len, bool &isError);
		// static size_t	_skipHeaderValue(const char *buff, size_t start, size_t len, bool &isError);
		static size_t	_parseHeaderCrlf(HTTPHeaders &httpHeaders, const char *buff, size_t start, size_t len);
		static size_t	_parseHeaderField(HTTPHeaders &httpHeaders, const char *buff, size_t start, size_t len);
		static size_t	_parseHeaderValue(HTTPHeaders &httpHeaders, const char *buff, size_t start, size_t len);

		static size_t	_parseBody(HTTPRequest &request, const char *buff, size_t start, size_t len);
		static size_t	_parseChunk(HTTPRequest &request, const char *buff, size_t start, size_t len);
		static size_t	_parseChunkData(HTTPRequest &request, const char *buff, size_t start, size_t len);
		static size_t	_parseMultipartForm(HTTPRequest &request, const char *buff, size_t start, size_t len);
		static size_t	_parseMultipartBody(HTTPRequest &request, const char *buff, size_t start, size_t len);
		static size_t	_parseRawBody(HTTPRequest &request, const char *buff, size_t start, size_t len);

		static size_t	_parseCgiBody(HTTPMessage &httpMessage, const char *buff, size_t start, size_t len);
		static size_t	_skipCrlf(HTTPParseState &parseState, const char *buff, size_t start, size_t len);
		static inline bool	_isCrlf(char current, char previous);
		static bool 		_notValidHeaderField(const int &c);
		static bool 		_notValidHeaderValue(const int &c);

		static const unsigned int	MAX_REQUEST_LINE_SIZE;
		static const unsigned int	MAX_METHOD_SIZE;
		static const unsigned int	MAX_HEADER_SIZE;
		static const unsigned int	MAX_BODY_SIZE;
};
/*

Dec  Char                           Dec  Char     Dec  Char     Dec  Char
---------                           ---------     ---------     ----------
  0  NUL (null)                      32  SPACE     64  @         96  `
  1  SOH (start of heading)          33  !         65  A         97  a
  2  STX (start of text)             34  "         66  B         98  b
  3  ETX (end of text)               35  #         67  C         99  c
  4  EOT (end of transmission)       36  $         68  D        100  d
  5  ENQ (enquiry)                   37  %         69  E        101  e
  6  ACK (acknowledge)               38  &         70  F        102  f
  7  BEL (bell)                      39  '         71  G        103  g
  8  BS  (backspace)                 40  (         72  H        104  h
  9  TAB (horizontal tab)            41  )         73  I        105  i
 10  LF  (NL line feed, new line)    42  *         74  J        106  j
 11  VT  (vertical tab)              43  +         75  K        107  k
 12  FF  (NP form feed, new page)    44  ,         76  L        108  l
 13  CR  (carriage return)           45  -         77  M        109  m
 14  SO  (shift out)                 46  .         78  N        110  n
 15  SI  (shift in)                  47  /         79  O        111  o
 16  DLE (data link escape)          48  0         80  P        112  p
 17  DC1 (device control 1)          49  1         81  Q        113  q
 18  DC2 (device control 2)          50  2         82  R        114  r
 19  DC3 (device control 3)          51  3         83  S        115  s
 20  DC4 (device control 4)          52  4         84  T        116  t
 21  NAK (negative acknowledge)      53  5         85  U        117  u
 22  SYN (synchronous idle)          54  6         86  V        118  v
 23  ETB (end of trans. block)       55  7         87  W        119  w
 24  CAN (cancel)                    56  8         88  X        120  x
 25  EM  (end of medium)             57  9         89  Y        121  y
 26  SUB (substitute)                58  :         90  Z        122  z
 27  ESC (escape)                    59  ;         91  [        123  {
 28  FS  (file separator)            60  <         92  \        124  |
 29  GS  (group separator)           61  =         93  ]        125  }
 30  RS  (record separator)          62  >         94  ^        126  ~
 31  US  (unit separator)            63  ?         95  _        127  DEL


  tchar          = "!" / "#" / "$" / "%" / "&" / "'" / "*"
                 / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
                 / DIGIT / ALPHA
*/

#endif