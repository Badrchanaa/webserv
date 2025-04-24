#ifndef __HTTPRESPONSE_HPP__
# define __HTTPRESPONSE_HPP__

#define READ EPOLLIN
#define WRITE EPOLLOUT
// #define ERROR EPOLLOUT | 

#include <string>
/*
POST /CGI/uploads
Host: 127.0.0.1


oeofewofowefoweof
temp_file

open(NON_BLOCKING);

EPOLL read(file);
EPOLL write(cgi);

EPOLL read(cgi);
EPOLL write(client_fd);
*/

class HTTPResponse
{
	public:
		typedef enum
		{
			CGI_WRITE,
			CGI_READ,
			SOCKET_WRITE,
			DONE
		} responseState;
		typedef enum
		{
			OK = 200,
			CREATED = 201,
			NOT_FOUND = 404,
		} statusCode
	
	public:
		HTTPResponse(void);
		HTTPResponse(const HTTPResponse &other);
		HTTPResponse& operator=(const HTTPResponse &other);
		~HTTPResponse();

		/// @brief initializes response from parsed request, spawns CGI process if 
		/// @param request 
		/// @param cgihandler 
		/// @param fd 
		void	init(HTTPRequest &request, CGIHandler *cgihandler, Config *config, int fd)
		{
		}
		// response is sent, IS DONE
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
		

		/// @brief resumes response processing. should be called on event notify.
		/// @param event EpollManager event.
		/// @return if should remove current event from list
		bool resume(bool isCgiReady, bool isClientReady)
		{
			// //
			// // CGIProcess
			// // 
			// if (this.has_cgi)
			// {
			// 	CGIProcess *cgi = handler.spawn(script, env, client_fd);
			// }
			// //	
			// manager.subscribe(fd, type, this, flags);
			// if (request.has_file() )
			// {
			// 	state = READ_FILE;
			// 	return;
			// 	// EPOLL ???
			// 	buffer[20];
			// 	int filefd = open(filename);
			// 	handler.edit_fd(fd, READ | EPOLLOUT, RSPONSE_NORMAL);
			// 	return;
			// 	// handler.add_read_epp(fd, READ);
			// 	/// ///
			// 	int bytes = read(filefd);
			// 	read_file();
			// 	response
				
			// }
			// // EPOLL ?????
			// 	cgi.handle_output();

			// int client_socket = handler.getresponsesocket(this);
			// int cgi_process_socket;
			// write(this->handler);
		}


	private:
		int	status_code;
		CGIProcess		*cgi;
		responseState	m_State;
		bool has_cgi;
		// int client_fd;

};

#endif
