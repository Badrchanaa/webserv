#include "HTTPMultipartForm.hpp"
#include <iostream>
#include <string>

HTTPMultipartForm::HTTPMultipartForm(HTTPMessage::header_map_t &mediaTypes)
{
	m_Boundary = mediaTypes["boundary"];
	m_ParseState.setReadBytes(0);
	m_ParseState.setState(HTTPParseState::PARSE_MULTIPART_BOUNDARY);
}

HTTPParseState&	HTTPMultipartForm::getParseState()
{
	return m_ParseState;
}

void			HTTPMultipartForm::onHeadersParsed()
{
	m_ParseState.setState(HTTPParseState::PARSE_BODY);
	FormPart&	currentPart = getCurrentPart();

	if (hasHeader("content-type"))
		currentPart.setContentType(getHeader("content-type"));
	if (hasHeader("content-disposition"))
		currentPart.setContentDisposition(getHeader("content-disposition"));
	std::cout << "part headers parsed" << std::endl;
	std::cout << "content disposition: " << currentPart.getContentDisposition() << std::endl;
	std::cout << "part addr: " << &currentPart << std::endl;

	m_Headers.clear();
}

void	HTTPMultipartForm::onNewPart()
{
	std::cout << " NEW PART " << std::endl;
	m_Parts.push_back(FormPart());
}

FormPart&				HTTPMultipartForm::getCurrentPart()
{
	return m_Parts.back();
}

HTTPMultipartForm::HTTPMultipartForm(const HTTPMultipartForm &other)
{
	*this = other;	
}

HTTPMultipartForm& HTTPMultipartForm::operator=(const HTTPMultipartForm &other)
{
	if (this == &other)
		return *this;
	// this->m_Boundary = other.m_Boundary;
	// this->m_Parts = other.m_Parts;
	return *this;
}

HTTPMultipartForm::~HTTPMultipartForm(void)
{
}