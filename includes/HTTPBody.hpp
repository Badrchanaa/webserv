#ifndef __HTTPBody_HPP__
# define __HTTPBody_HPP__

#include <string>
#include <vector>
#include <stdint.h>
#include <fstream>

# define MB (1024U * 1024U)

# define MAX_BODY_MEMORY (2 * MB)

class HTTPBody
{
	public:
		HTTPBody();
		HTTPBody(char *buffer, size_t len);
		~HTTPBody();

		bool	append(const char *buffer, size_t len);
		void	flush();

	private:
		bool	_switchToFile();
		bool	_writeToBuffer(const char *buffer, size_t len);
		bool	_writeToFile(const char *buffer, size_t len);

		std::string				m_Filename;
		std::ofstream			m_File;
		std::vector<char>		m_VectorBuffer;
		size_t					m_Size;
		bool					m_IsFile;
};

#endif