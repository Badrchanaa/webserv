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
			std::cout << "form part cp" << std::endl;
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

		std::string	getContentDisposition()
		{
			return m_ContentDisposition;
		}
		std::string	getContentType()
		{
			return m_ContentType;
		}
		HTTPBody&	getBody()
		{
			return m_Body;
		}
	private:
		std::string m_ContentType;
		std::string m_ContentDisposition;
		HTTPBody	m_Body;
};

class HTTPMultipartForm: public HTTPHeaders
{
	public:
		virtual HTTPParseState&	getParseState();

		virtual void			onHeadersParsed();
		void					onNewPart();
		FormPart&				getCurrentPart();
		HTTPMultipartForm(HTTPMessage::header_map_t &mediaTypes);
		HTTPMultipartForm(const HTTPMultipartForm &other);
		HTTPMultipartForm& operator=(const HTTPMultipartForm &other);
		std::string getBoundary()
		{
			return m_Boundary;
		}
		size_t	getPartsCount()
		{
			return m_Parts.size();
		}
		std::vector<FormPart *>&	getParts()
		{
			return m_Parts;
		}
		// void	newPart();
		virtual ~HTTPMultipartForm();
	private:
		std::string				m_Boundary;
		std::vector<FormPart *> 	m_Parts;
		HTTPParseState			m_ParseState;
};

#endif