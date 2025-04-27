#ifndef __HTTPRESPONSE_HPP__
# define __HTTPRESPONSE_HPP__

#define READ EPOLLIN
#define WRITE EPOLLOUT
// #define ERROR EPOLLOUT | 

#include "CGIHandler.hpp"
#include "Config.hpp"
#include "HTTPRequest.hpp"
#include "CGIHandler.hpp"

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
		} statusCode;
	
	public:
		HTTPResponse(void){}
		HTTPResponse(const HTTPResponse &other);
		HTTPResponse& operator=(const HTTPResponse &other);
		~HTTPResponse(){}

		/// @brief initializes response from parsed request, spawns CGI process if 
		/// @param request 
		/// @param cgihandler 
		/// @param fd 
		void	init(HTTPRequest &request, CGIHandler &cgihandler, Config &config, int fd)
		{
			clientFd = fd;
			this->request = &request;
			(void)cgihandler;
			(void)config;
		}
		// response is sent, IS DONE
		void		reset()
		{
			return;
		}

		bool		isDone() const
		{
			return true;
		}
		// keep alive or close
		bool		isKeepAlive() const
		{
			return false;
		}
		// if response has cgi
		bool		hasCgi() const
		{
			return false;
		}
		// get response status
		statusCode	getStatus()
		{
			return OK;
		}

		// returns response state (CGI_READ, CGI_WRITE, SOCKET_WRITE, DONE)
		// use it to manage epoll events for this response, socket or cgi.
		responseState getState() const
		{
			return SOCKET_WRITE;
		}

		// void				reset()
		// {
		// 	return;
		// }
		

		/// @brief resumes response processing. should be called on event notify.
		/// @param event EpollManager event.
		/// @return if should remove current event from list
		bool resume(bool isCgiReady, bool isClientReady)
		{
			std::stringstream responseStream;
			if (isCgiReady)
				std::cout << "CGI READY" << std::endl;
			if (isClientReady)
				std::cout << "CLIENT READY" << std::endl;
			responseStream << "<html><body>";
			responseStream << "<style>td{padding: 10;border: 2px solid black;background: #ababed;font-size:16;font-style:italic;}</style>";
			responseStream << "request.path: '" << request->getPath() << "'" << std::endl;
			responseStream << "<table>";
			responseStream << "<h1>REQUEST HEADERS</h1>";
			HTTPRequest::HeaderMap headers = request->getHeaders();
			for (HTTPRequest::HeaderMap::iterator it = headers.begin(); it != headers.end(); it++)
			{
				responseStream << "<tr>";
				responseStream << "<td>" << it->first << "</td>";
				responseStream << "<td>" << it->second << "</td>";
				responseStream << "</tr>";
			}
			responseStream << "</table>";
			responseStream << "</body></html>";
			responseStream << "<h1>REQUEST BODY:</h1>";

			std::string res;
			size_t	resLen = responseStream.str().length();

			std::stringstream ss;

			ss << resLen;
			std::string resLenStr;
			ss >> resLenStr;
			res += "HTTP/1.1 200 OK\r\n";
			res += "Content-length:";
			res += resLenStr;
			res += "\r\n";
			res += "\r\n";
			res += responseStream.str();

			
			size_t bufflen = res.length();
			const char *buff = res.c_str();
			write(clientFd, buff, bufflen);
			std::cout << "RESPONSE SENT" << std::endl;
			
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
			return true;
		}


	private:
		int	clientFd;
		int	status_code;
		HTTPRequest *request;
		CGIProcess		*cgi;
		responseState	m_State;
		bool has_cgi;
		// int client_fd;

};

#endif