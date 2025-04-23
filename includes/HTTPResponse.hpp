#ifndef __HTTPRESPONSE_HPP__
# define __HTTPRESPONSE_HPP__

#define READ EPOLLIN
#define WRITE EPOLLOUT
// #define ERROR EPOLLOUT | 

#include <string>
#include "http/HTTPRequest.hpp"
#include ""
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
		HTTPResponse(void);
		HTTPResponse(const HTTPResponse &other);
		HTTPResponse& operator=(const HTTPResponse &other);
		~HTTPResponse();

		void	init(HTTPRequest &request, CGIHander *cgihandler, int fd)
		{
			process_headers();
			if (request.path == "/CGI")
			{
				has_cgi = true;
			}
			process();
		}

		bool	buffer_has_data()
		{

		}
		void process()
		{
			//
			// CGIProcess
			// 
			if (this.has_cgi)
			{
				CGIProcess *cgi = handler.spawn(script, env, client_fd);
			}
			//	
			manager.subscribe(fd, type, this, flags);
			if (request.has_file() )
			{
				state = READ_FILE;
				return;
				// EPOLL ???
				buffer[20];
				int filefd = open(filename);
				handler.edit_fd(fd, READ | EPOLLOUT, RSPONSE_NORMAL);
				return;
				// handler.add_read_epp(fd, READ);
				/// ///
				int bytes = read(filefd);
				read_file();
				response
				
			}
			// EPOLL ?????
				cgi.handle_output();

			int client_socket = handler.getresponsesocket(this);
			int cgi_process_socket;
			write(this->handler);
		}

		void resume(int fd, type)
		{
		}

		void	read_file()
		{

		}
		void	write_file()
		{

		}
		read_cgi()
		{

		}
		write_cgi(){
		}

		bytes_read = Connection.read(int fd, &buff, size_t len);
		notify()

		Connection.write();

	private:
		int	status_code;
		CGIProcess *cgi;
		CGIHandler	*cgi_handler;
		bool has_cgi;
		// int client_fd;

};

#endif