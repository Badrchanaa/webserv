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
#include "HTTPMessage.hpp"

typedef enum transferEncoding
{
	DEFAULT,
	CHUNKED,
} transferEncoding;

typedef enum requestError
{
	ERR_NONE,
	ERR_UNIMPLEMENTED_TE,
	ERR_INVALID_HOST,
	ERR_INVALID_METHOD,
	ERR_INVALID_PATH,
	ERR_INVALID_CONTENT_LENGTH,

} requestError;

class HTTPRequest: public HTTPMessage
{

	public:
		HTTPRequest(std::vector<ConfigServer> &servers);
		// HTTPRequest(const HTTPRequest &other);
		HTTPRequest& operator=(const HTTPRequest &other);
		~HTTPRequest();
		HTTPParseState		&getParseState();
		void				setMethod(const char *method_cstr);
		httpMethod			getMethod() const;
		const char*			getMethodStr() const;
		void				appendToPath(const char *buff, size_t start, size_t len);
		// bool				appendBody(const char *buff, size_t len);
		bool				validPath();
		const std::string	getQuery() const;
		// std::string			getBodyStr() const;
		const std::string&	getPath() const;
		bool				isTransferChunked() const;
		bool				isMultipartForm() const;
		bool				isComplete() const;
		bool				isError() const;
		void				processHeaders();
		const ConfigServer*	getServer() const;
		void				reset();

	private:
		bool				_validateHeaders();
		bool				_checkTransferChunked();
		bool				_preBody();

		httpMethod			m_Method;
		HTTPParseState		m_ParseState;
		requestError		m_Error;
		std::string			m_Host;
		std::string			m_Path;
		transferEncoding	m_TransferEncoding;
		std::string			m_Query;
		HTTPMultipartForm	*m_MultipartForm;
		/*
			TODO: remove ConfigServers from constructor, get them from config after header parsing
		*/
		std::vector<ConfigServer>		&m_ConfigServers;
		const ConfigServer				*m_ConfigServer;
};

#endif