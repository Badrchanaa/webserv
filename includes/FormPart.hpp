#ifndef __FORMPART_HPP__
#define __FORMPART_HPP__

#include "HTTPBody.hpp"
#include "HTTPHeaders.hpp"

class FormPart
{
	public:
		FormPart(void);
		FormPart(const FormPart &part);
		~FormPart();

		void	setContentType(const std::string &contentType);
		void	setContentDisposition(const std::string &contentDisposition);
		HTTPHeaders::header_map_t	getDispositionDirectives();
		std::string	getContentDisposition();
		std::string	getContentType();
		HTTPBody&	getBody();
	private:
		std::string m_ContentType;
		std::string m_ContentDisposition;
		HTTPBody	m_Body;
};

#endif