#ifndef __HTTPREQUEST_HPP__
# define __HTTPREQUEST_HPP__

#include "HTTPParseState.hpp"
#include "HTTPMultipartForm.hpp"
#include "HTTPBody.hpp"
#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include <list>
#include "Config.hpp"

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
		HTTPRequest(std::vector<ConfigServer> &servers);
		// HTTPRequest(const HTTPRequest &other);
		HTTPRequest& operator=(const HTTPRequest &other);
		~HTTPRequest();
		HTTPParseState		&getParseState();
		void				setMethod(const char *method_cstr);
		const char*			getMethodStr() const;
		void				appendToPath(const char *buff, size_t start, size_t len);
		bool				appendBody(const char *buff, size_t len);
		const HTTPBody&		getBody() const;
		bool				validPath();
		size_t				getContentLength() const;
		void				addHeader(std::string key, std::string value);
		std::string			getHeader(std::string &key) const;
		std::string			getHeader(const char *key) const;
		// std::string			getBodyStr() const;
		inline bool			hasHeader(const char *key) const;
		const HeaderMap&	getHeaders() const;
		const std::string&	getPath() const;
		bool				isTransferChunked() const;
		bool				isMultipartForm() const;
		bool				isComplete() const;
		bool				isError() const;
		void				processHeaders();
		ConfigServer		*getServer() const;
		void				reset();

	private:
		bool				_validateHeaders();
		bool				_checkTransferChunked();
		bool				_preBody();

		httpMethod			m_Method;
		HTTPParseState		m_ParseState;
		requestError		m_Error;
		HeaderMap			m_Headers;
		HTTPBody			m_Body;
		std::string			m_Host;
		std::string			m_Path;
		size_t				m_ContentLength;
		transferEncoding	m_TransferEncoding;
		HTTPMultipartForm	*m_MultipartForm;
		/*
			TODO: remove ConfigServers from constructor, get them from config after header parsing
		*/
		std::vector<ConfigServer>		&m_ConfigServers;
		ConfigServer		*m_ConfigServer;
};

#endif