#ifndef __HTTPMESSAGE_HPP__
# define __HTTPMESSAGE_HPP__

#include "http.hpp"
#include "HTTPParseState.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPBody.hpp"

#define CRLF "\r\n"


class HTTPMessage: public HTTPHeaders
{
	public:
		virtual void			onBodyDone() = 0;
		size_t					getContentLength() const;
		HTTPBody&				getBody();
		bool					appendBody(const char *buff, size_t len);
		bool					appendBody(const std::string str);
		bool					appendBody(const std::vector<char>& vec);
		bool					isParseComplete() const;
		virtual HTTPParseState	&getParseState();
	public:
		HTTPMessage(void);
		virtual ~HTTPMessage();
	protected:
		HTTPBody			m_Body;
		size_t				m_ContentLength;
		HTTPParseState		m_ParseState;
};

#endif