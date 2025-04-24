#ifndef __HTTPREQUEST_HPP__
# define __HTTPREQUEST_HPP__

#include "HTTPParseState.hpp"
#include <map>
#include <vector>
#include <string>

typedef enum
{
	ERR_NONE,
	ERR_INVALID_HOST,
	ERR_INVALID_METHOD,
	ERR_INVALID_PATH,
	ERR_INVALID_CONTENT_LENGTH,

} RequestError;

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
		void			addHeader(std::string &key, std::string &value);
		bool			processHeaders();
		std::string		getHeader(std::string &key) const;
		const std::string	&getPath() const;
		bool			isChunked();
	private:
		HTTPParseState		m_ParseState;
		RequestError		m_Error;
		HeaderMap			m_Headers;
		std::vector<char>	m_Body;
		std::string			m_Host;
		std::string			m_Path;
		uint64_t			m_ContentLength;
		bool				m_TransferChunked;
		// httpMethod			m_Method;
		// size_t				m_ContentLength;
};

#endif