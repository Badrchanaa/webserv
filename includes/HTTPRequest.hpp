#ifndef __HTTPREQUEST_HPP__
# define __HTTPREQUEST_HPP__

#include "HTTPParseState.hpp"
#include "HTTPMultipartForm.hpp"
#include "HTTPBody.hpp"
#include <map>
#include <vector>
#include <string>
#include <stdint.h>

typedef enum transferEncoding
{
	DEFAULT,
	CHUNKED,
} transferEncoding;

typedef enum requestError
{
	ERR_NONE,
	ERR_INVALID_HOST,
	ERR_INVALID_METHOD,
	ERR_INVALID_PATH,
	ERR_INVALID_CONTENT_LENGTH,

} requestError;

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
		HTTPParseState		&getParseState();
		void				setMethod(const char *method_cstr);
		void				appendToPath(const char *buff, size_t start, size_t len);
		bool				appendBody(const char *buff, size_t len);
		bool				validPath();
		void				addHeader(std::string &key, std::string &value);
		std::string			getHeader(std::string &key) const;
		const std::string	&getPath() const;
		bool				isTransferChunked() const;
		bool				isMultipartForm() const;
		bool				isComplete() const;
		void				processHeaders();

	private:
		bool				_validateHeaders();
		bool				_preBody();

		httpMethod			m_Method;
		HTTPParseState		m_ParseState;
		requestError		m_Error;
		HeaderMap			m_Headers;
		HTTPBody			m_Body;
		std::string			m_Host;
		std::string			m_Path;
		uint64_t			m_ContentLength;
		transferEncoding	m_TransferEncoding;
		HTTPMultipartForm	*m_MultipartForm;
};

#endif