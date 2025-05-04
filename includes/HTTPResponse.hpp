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
			CGI_WRITE = 1,
			CGI_READ = 2,
			PROCESS_BODY,
			HEADERS = 3,
			SOCKET_WRITE = 5,
			DONE = 6
		} responseState;
		typedef enum
		{
			OK = 200,
			CREATED = 201,
			BAD_REQUEST = 400,
			FORBIDDEN = 403,
			NOT_FOUND = 404,
			SERVER_ERROR = 500

		} statusCode;
	
	public:
		HTTPResponse( void );
		// HTTPResponse(const HTTPResponse &other);
		// HTTPResponse& operator=(const HTTPResponse &other);
		~HTTPResponse();

		/// @brief initializes response from parsed request
		/// @param request 
		/// @param cgihandler 
		/// @param fd 
		void	init(HTTPRequest &request, CGIHandler &cgihandler, ConfigServer *configServer, int fd);


		// response is sent, IS DONE
		void		reset();

		bool		isDone() const;
		// keep alive or close
		bool		isKeepAlive() const;
		// if response has cgi
		bool		hasCgi() const;
		// get response status
		statusCode	getStatus();

		// returns response state (CGI_READ, CGI_WRITE, SOCKET_WRITE, DONE)
		// use it to manage epoll events for this response, socket or cgi.
		responseState getState() const;

		// void				reset()
		// {
		// 	return;
		// }
		void	sendHeaders();
		void	processBody();
		void	sendBody();
		/// @brief resumes response processing. should be called on event notify.
		/// @param event EpollManager event.
		/// @return if should remove current event from list
		bool resume(bool isCgiReady, bool isClientReady);

	private:
		const std::string	_statusToString() const;
		bool	_isCgiPath(std::string path, ConfigServer *configServer);
		void	_initCgi(std::string path, CGIHandler &cgihandler, ConfigServer *configServer);
		void	_initBadRequest();

		int				m_ClientFd;
		statusCode		m_StatusCode;
		HTTPBody		m_Body;
		HTTPRequest*	m_Request;
		// CGIProcess*		m_Cgi;
		responseState	m_State;
		// ConfigServer*	m_ConfigServer;
		size_t			m_CursorPos;
		bool			m_HasCgi;
		// int client_fd;

};

#endif