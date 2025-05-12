#ifndef __HTTPRESPONSE_HPP__
# define __HTTPRESPONSE_HPP__

#define READ EPOLLIN
#define WRITE EPOLLOUT

#include "CGIHandler.hpp"
#include "Config.hpp"
#include "HTTPMessage.hpp"
#include "HTTPRequest.hpp"
#include "CGIHandler.hpp"

#include <string>

class HTTPResponse: public HTTPMessage
{
	public:
		typedef enum
		{
			INIT = 0,
			PROCESS_BODY,
			PROCESS_HEADERS,
			SEND_HEADERS,
			SEND_BODY,
			DONE
		}	responseState;

		typedef enum
		{
			CGI_WRITE = 1,
			CGI_READ,
			SOCKET_WRITE,
		}	pollState;

		typedef enum
		{
			OK = 200,
			CREATED = 201,
			BAD_REQUEST = 400,
			FORBIDDEN = 403,
			NOT_FOUND = 404,
			SERVER_ERROR = 500,
			NOT_IMPLEMENTED = 501,
		}	statusCode;
	
	public:
		HTTPResponse( void );
		// HTTPResponse(const HTTPResponse &other);
		// HTTPResponse& operator=(const HTTPResponse &other);
		~HTTPResponse();

		/// @brief initializes response from parsed request
		/// @param request /// @param cgihandler 
		/// @param fd 
		void	init(const HTTPRequest &request, const CGIHandler &cgihandler, const ConfigServer *configServer, int fd);


		// response is sent, IS DONE
		void		reset();

		bool		isDone() const;
		// keep alive or close
		bool		isKeepAlive() const;
		// if response has cgi
		bool		hasCgi() const;
		// get response status
		statusCode	getStatus();

		// use it to manage epoll events for this response, socket or cgi.
		pollState		getPollState() const;

		// response processing state
		responseState	getState() const;

		// void				reset()
		// {
		// 	return;
		// }
		/// @brief resumes response processing. should be called on event notify.
		/// @param event EpollManager event.
		/// @return if should remove current event from list
		bool resume(bool isCgiReady, bool isClientReady);

	private:
		void	_sendHeaders();
		void	_processBody();
		void	_debugBody();
		void	_processHeaders();
		void	_sendBody();
		const std::string	_statusToString() const;
		bool	_isCgiPath(const std::string path, const ConfigServer *configServer);
		void	_initCgi(const std::string path, const CGIHandler &cgihandler, const ConfigServer *configServer);
		void	_initBadRequest();

		std::stringstream	m_HeadersStream;
		int					m_ClientFd;
		statusCode			m_StatusCode;
		const HTTPRequest*	m_Request;
		// CGIProcess*		m_Cgi;
		responseState		m_State;
		pollState			m_PollState;
		// ConfigServer*	m_ConfigServer;
		size_t				m_CursorPos;
		bool				m_HasCgi;
		std::string			m_ResourcePath;
		// int client_fd;

};

#endif