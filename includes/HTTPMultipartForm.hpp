#ifndef __HTTPMULTIPARTFORM_HPP__
# define __HTTPMULTIPARTFORM_HPP__

#include <HTTPMessage.hpp>
#include <string>

class FormPart
{
	public:
		FormPart(void) //m_Body(), m_ContentType(), m_ContentDisposition()
		{

		}
		FormPart(const FormPart &part)
		{
			(void)part;
			return;
		}
		~FormPart()
		{

		}
		void	setContentType(std::string &contentType);
		void	setContentDisposition(std::string &contentDisposition);
		std::string	getContentType();
		std::string	getContentDisposition();
		HTTPBody&	getBody();
	private:
		std::string m_ContentType;
		std::string m_ContentDisposition;
		HTTPBody	m_Body;
};
class HTTPMultipartForm
{
	public:
		HTTPMultipartForm(HTTPMessage::header_map_t &mediaTypes);
		HTTPMultipartForm(const HTTPMultipartForm &other);
		HTTPMultipartForm& operator=(const HTTPMultipartForm &other);
		void	addPart(FormPart& part);
		~HTTPMultipartForm();
	private:
		std::string				m_Boundary;
		std::vector<FormPart> 	m_Parts;
		
};

#endif