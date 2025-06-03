#ifndef __HTTPMULTIPARTFORM_HPP__
# define __HTTPMULTIPARTFORM_HPP__

#include <HTTPMessage.hpp>
#include <string>

class FormPart
{
	public:
		FormPart(void)
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
		void	setContentType(const std::string &contentType)
		{
			m_ContentType = contentType;
		}
		void	setContentDisposition(const std::string &contentDisposition)
		{
			m_ContentDisposition = contentDisposition;
		}
		std::string	getContentType();
		std::string	getContentDisposition();
		HTTPBody&	getBody();
	private:
		std::string m_ContentType;
		std::string m_ContentDisposition;
		HTTPBody	m_Body;
};

class HTTPMultipartForm: HTTPHeaders
{
	public:
		virtual HTTPParseState&	getParseState();

		virtual void			onHeadersParsed();
		void					onPartParsed();
		FormPart&				getCurrentPart();
		HTTPMultipartForm(HTTPMessage::header_map_t &mediaTypes);
		HTTPMultipartForm(const HTTPMultipartForm &other);
		HTTPMultipartForm& operator=(const HTTPMultipartForm &other);
		// void	newPart();
		~HTTPMultipartForm();
	private:
		std::string				m_Boundary;
		std::vector<FormPart> 	m_Parts;
		HTTPParseState			m_ParseState;
		
};

#endif