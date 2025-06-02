#ifndef __HTTPMESSAGE_HPP__
# define __HTTPMESSAGE_HPP__

#include <string>
#include <map>
#include "http.hpp"
#include "HTTPParseState.hpp"
#include "HTTPBody.hpp"

#define CRLF "\r\n"

typedef enum transferEncoding
{
	DEFAULT,
	CHUNKED,
} transferEncoding;

class HTTPMessage
{
	public:
		typedef std::map<std::string, std::string> header_map_t;

		virtual void		onHeadersParsed() = 0;
		virtual void		onBodyDone() = 0;
		bool				hasHeader(const char *key) const;
		size_t				getContentLength() const;
		void				addHeader(std::string key, std::string value);
		void				addHeader(std::string key, size_t value);
		bool				removeHeader(const char *key);
		const header_map_t&	getHeaders() const;
		std::string			getHeader(std::string &key) const;
		std::string			getHeader(const char *key) const;
		HTTPBody&			getBody();
		bool	            appendBody(const char *buff, size_t len);
		bool	            appendBody(const std::string str);
		bool	            appendBody(const std::vector<char>& vec);
		bool				isTransferChunked() const;
		HTTPParseState		&getParseState();
		bool				isParseComplete() const;
	public:
		HTTPMessage(void);
		~HTTPMessage();
	protected:
		HTTPBody			m_Body;
		header_map_t		m_Headers;
		size_t				m_ContentLength;
		HTTPParseState		m_ParseState;
		transferEncoding	m_TransferEncoding;
};

#endif