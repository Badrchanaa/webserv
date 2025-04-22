#ifndef __HTTPREQUEST_HPP__
# define __HTTPREQUEST_HPP__

#include "HTTPParseState.hpp"
#include <map>
#include <vector>
#include <string>

class HTTPRequest
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
	public:
		HTTPRequest(void);
		HTTPRequest(const HTTPRequest &other);
		HTTPRequest& operator=(const HTTPRequest &other);
		~HTTPRequest();
		HTTPParseState	&getParseState();
		void			setMethod(char *method_cstr);
		void			appendToPath(char *buff, size_t start, size_t len);
		bool			validPath();
		const std::string		&getPath() const;
	private:
		HTTPParseState		m_ParseState;
		HeaderMap			m_Headers;
		std::vector<char>	m_Body;
		std::string			m_Host;
		std::string			m_Path;
		// httpMethod			m_Method;
		// size_t				m_ContentLength;
};

#endif