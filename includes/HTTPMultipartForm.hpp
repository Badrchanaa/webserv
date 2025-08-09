#ifndef __HTTPMULTIPARTFORM_HPP__
# define __HTTPMULTIPARTFORM_HPP__

#include "HTTPMessage.hpp"
#include <string>

#include "FormPart.hpp"

class HTTPMultipartForm: public HTTPHeaders
{
	public:
		virtual HTTPParseState&	getParseState();

		virtual void			onHeadersParsed();
		void					onParseComplete();
		void					onNewPart();
		FormPart&				getCurrentPart();
		FormPart*				getFirstFilePart();

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
