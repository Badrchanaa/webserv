#ifndef __HTTPMESSAGE_HPP__
# define __HTTPMESSAGE_HPP__

#include <string>
#include <map>
#include "HTTPBody.hpp"

#define CRLF "\r\n"

class HTTPMessage
{
	public:
		typedef std::map<std::string, std::string> HeaderMap;
		typedef enum
		{
			GET,
			POST,
			PUT,
			DELETE,
		} httpMethod;
		inline bool			hasHeader(const char *key) const;
		size_t				getContentLength() const;
		void				addHeader(std::string key, std::string value);
		void				addHeader(std::string key, size_t value);
		const HeaderMap&	getHeaders() const;
		std::string			getHeader(std::string &key) const;
		std::string			getHeader(const char *key) const;
		const HTTPBody&		getBody() const;
		bool	            appendBody(const char *buff, size_t len);
		bool	            appendBody(const std::string str);
		bool	            appendBody(const std::vector<char>& vec);
	public:
		HTTPMessage(void);
		~HTTPMessage();
	protected:
		HTTPBody			m_Body;
		HeaderMap			m_Headers;
		size_t				m_ContentLength;
};

#endif