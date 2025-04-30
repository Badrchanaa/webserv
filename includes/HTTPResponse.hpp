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
		HTTPResponse() {
			m_BodyProcessed = false;
			m_BodySent = false;
			m_CursorPos = 0;
			m_HeadersSent = false;
		}
		HTTPResponse(const HTTPResponse &other);
		HTTPResponse& operator=(const HTTPResponse &other);
		~HTTPResponse(){}

		/// @brief initializes response from parsed request
		/// @param request 
		/// @param cgihandler 
		/// @param fd 
		void	init(HTTPRequest &request, CGIHandler &cgihandler, ConfigServer *configServer, int fd)
		{
			m_ClientFd = fd;
			this->request = &request;
			(void)cgihandler;
			(void)configServer;
		}

		void	appendBody(const char *buff, size_t start, size_t end)
		{
			m_Body.append(buff + start, end);	
		}

		void	appendBody(const std::string str)
		{
			size_t		size;
			const char	*strBuff;

			size = str.length();
			strBuff = str.c_str();
			m_Body.append(strBuff , size);
		}

		void	appendBody(const std::vector<char>& vec)
		{
			m_Body.append(&vec[0], vec.size());
		}

		// response is sent, IS DONE
		void		reset()
		{
			return;
		}

		bool		isDone() const
		{
			return m_BodySent;
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

		void	sendHeaders()
		{
			std::string res;
			std::stringstream ss;

			ss << m_Body.getSize();
			std::string resLenStr;
			ss >> resLenStr;
			res += "HTTP/1.1 200 OK\r\n";
			res += "Content-length:";
			res += resLenStr;
			res += "\r\n";
			res += "\r\n";
			write(m_ClientFd, res.c_str(), res.length());
			m_HeadersSent = true;
		}

		void	processBody()
		{
			m_CursorPos = 0;
			std::stringstream responseStream;
			responseStream << "<html><body>";
			responseStream << "<style>td{padding: 10;border: 2px solid black;background: #ababed;font-size:16;font-style:italic;}</style>";
			if (request->isError())
				responseStream << "<h4>PARSE ERROR</h4>";
			else
				responseStream << "<h4>PARSE SUCCESS</h4>";
			responseStream << "<h4>method: " << request->getMethodStr() << "<h4>";
			responseStream << "request path: '" << request->getPath() << "'" << std::endl;
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
			appendBody(responseStream.str());
			if (!request->isError())
			{
			appendBody(request->getBody().getBuffer());
			std::cout << "BODY PROCESSING END (size:" << m_Body.getSize() << ")" << std::endl;
			}
			m_BodyProcessed = true;
		}

		void	sendBody()
		{
			const std::vector<char>	&bodyBuffer = m_Body.getBuffer();
			const char * buff = static_cast<const char *>(&bodyBuffer[m_CursorPos]);
			std::size_t	size = bodyBuffer.size();
			// write(1, buff, size);

			ssize_t	bytesWritten = send(m_ClientFd, buff, size - m_CursorPos, 0);
			if (bytesWritten < 0)
			{
				 m_BodySent = true;
				 return ;
			}
			std::cout << "written: " << bytesWritten << " cursorPos: " << m_CursorPos << std::endl;
			if (bytesWritten + m_CursorPos < size)
			{
				m_CursorPos = bytesWritten;
				std::cout << "written: " << bytesWritten << " cursorPos: " << m_CursorPos << std::endl;
			}
			else
				m_BodySent = true;
		}

		/// @brief resumes response processing. should be called on event notify.
		/// @param event EpollManager event.
		/// @return if should remove current event from list
		bool resume(bool isCgiReady, bool isClientReady)
		{
			if (isCgiReady)
				std::cout << "CGI READY" << std::endl;
			if (isClientReady)
				std::cout << "CLIENT READY" << std::endl;
			if (!m_BodyProcessed)
				processBody();
			if (isClientReady && !m_HeadersSent)
			{
				sendHeaders();
				return false;
			}
			if (isClientReady && !m_BodySent)
				sendBody();
			if (m_BodySent)
				std::cout << "RESPONSE COMPLETE" << std::endl;	
			else
				std::cout << "RESPONSE ONGOING" << std::endl;	
			return m_BodySent;
		}


	private:
		int			m_ClientFd;
		int			m_StatusCode;
		size_t		m_CursorPos;
		HTTPBody	m_Body;
		HTTPRequest	*request;
		CGIProcess	*cgi;
		responseState	m_State;
		ConfigServer	*m_ConfigServer;
		bool	m_HasCgi;
		bool	m_HeadersSent;
		bool	m_BodyProcessed;
		bool	m_BodySent;
		// int client_fd;

};

#endif