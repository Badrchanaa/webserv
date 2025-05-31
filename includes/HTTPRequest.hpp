#ifndef __HTTPREQUEST_HPP__
# define __HTTPREQUEST_HPP__

#include "HTTPMultipartForm.hpp"
#include "HTTPBody.hpp"
#include <map>
#include <vector>
#include <string>
#include <stdint.h>
#include <list>
#include "Config.hpp"
#include "HTTPMessage.hpp"

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
		void				setMethod(const char *method_cstr);
		httpMethod			getMethod() const;
		const char*			getMethodStr() const;
		void				appendUri(const char *buff, size_t start, size_t len);
		// bool				appendBody(const char *buff, size_t len);
		bool				validUri();
		// std::string			getBodyStr() const;
		const std::string&	getPath() const;
		const std::string&	getUri() const;
		bool				isMultipartForm() const;
		bool				isError() const;
		const ConfigServer*	getServer() const;
		void				reset();
		virtual void		onHeadersParsed();
		virtual void		onBodyDone();

		// added by regex 
		const std::string	getQuery() const{
			return this->m_Query;
		}
	private:
		void				test();
		void				_checkMultipart();
		bool				_checkTransferChunked();
		bool				_validateHeaders();
		bool				_preBody();

		httpMethod			m_Method;
		requestError		m_Error;
		std::string			m_Host;
		std::string			m_Path;
		std::string			m_Query;
		std::string			m_Uri;
		HTTPMultipartForm	*m_MultipartForm;
		/*
			TODO: remove ConfigServers from constructor, get them from config after header parsing
		*/
		std::vector<ConfigServer>		&m_ConfigServers;
		const ConfigServer				*m_ConfigServer;


};

#endif