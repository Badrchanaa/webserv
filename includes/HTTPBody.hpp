#ifndef __HTTPBody_HPP__
# define __HTTPBody_HPP__

#include <string>
#include <vector>
#include <stdint.h>
#include <stream>

# define MB (1024U * 1024U)

# define MAX_BODY_MEMORY (2 * MB)

class BodyBuffer: public std::stream_buf
{

}

class HTTPBody
{
	public:
		HTTPBody();
		HTTPBody(char *buffer, size_t len);
		~HTTPBody();

		void	operator<<(std::string);
		void	append(const char *buffer, size_t len);

	private:
		std::string				m_Swapfilename;
		std::vector<uint8_t>	m_Content;
		size_t					m_Size;
		bool					m_IsFile;
		std::stream *r;
};

#endif