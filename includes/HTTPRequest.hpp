#ifndef __HTTPREQUEST_HPP__
#define __HTTPREQUEST_HPP__

#include "Config.hpp"
#include "HTTPBody.hpp"
#include "HTTPMessage.hpp"
#include "HTTPMultipartForm.hpp"
#include <list>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

typedef enum requestError
{
    ERR_NONE,
    ERR_UNIMPLEMENTED_TE,
    ERR_INVALID_HOST,
    ERR_INVALID_METHOD,
    ERR_INVALID_PATH,
    ERR_INVALID_CONTENT_LENGTH,
    ERR_CONTENT_TOO_LARGE,
} requestError;

typedef enum transferEncoding
{
    DEFAULT,
    CHUNKED,
} transferEncoding;

class HTTPRequest : public HTTPMessage
{
	public:
		void forceCleanup() {
			m_Path.clear();
			m_Headers.clear();
			
			// Force string deallocation
			std::string empty_path;
			m_Path.swap(empty_path);
			
			std::string empty_uri;
			m_Uri.swap(empty_uri);
			
			std::string empty_query;
			m_Query.swap(empty_query);
			
			// Force map deallocation
			header_map_t empty_headers;
			m_Headers.swap(empty_headers);
			
			if (multipartForm) {
				delete multipartForm;
				multipartForm = NULL;
			}
		}
		HTTPRequest(std::vector<ConfigServer> &servers);
		// HTTPRequest(const HTTPRequest &other);
		HTTPRequest& operator=(const HTTPRequest &other);
		virtual ~HTTPRequest();
		virtual void		onHeadersParsed();
		virtual void		onBodyDone();

	void setMethod(const char *method_cstr);
	httpMethod getMethod() const;
	const char *getMethodStr() const;
	void appendUri(const char *buff, size_t start, size_t len);
	// bool				appendBody(const char *buff, size_t
	// len);
	bool validUri();
	// std::string			getBodyStr() const;
	const std::string &getPath() const;
	const std::string &getUri() const;
	HTTPMultipartForm *getMultipartForm();
	bool isMultipartForm() const;
	bool isError() const;
	void setError(requestError err)
	{
	    m_Error = err;
	    m_ParseState.setError();
	}
	requestError getError() const { return m_Error; }

	const ConfigServer *getServer() const;
	void reset();
	bool isTransferChunked() const;

	// added by regex
	const std::string getQuery() const { return this->m_Query; }

	HTTPMultipartForm *multipartForm;

    private:
	void _checkMultipart();
	bool _checkTransferChunked();
	bool _validateHeaders();
	bool _preBody();

	bool m_isMultipartForm;
	std::string m_FormBoundary;

	httpMethod m_Method;
	requestError m_Error;
	std::string m_Host;
	std::string m_Path;
	std::string m_Query;
	std::string m_Uri;
	transferEncoding m_TransferEncoding;
	size_t m_BodySize;
	size_t m_BoundaryIndex;

	/*
	        TODO: remove ConfigServers from constructor, get them from config
	   after header parsing
	*/
	std::vector<ConfigServer> &m_ConfigServers;
	const ConfigServer *m_ConfigServer;
};

#endif
