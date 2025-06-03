#include "HTTPMultipartForm.hpp"
#include <iostream>
#include <string>

HTTPMultipartForm::HTTPMultipartForm(HTTPMessage::header_map_t &mediaTypes)
{
	m_Boundary = mediaTypes["boundary"];
}

HTTPParseState&	HTTPMultipartForm::getParseState()
{
	return m_ParseState;
}

void			HTTPMultipartForm::onHeadersParsed()
{
}

void	HTTPMultipartForm::onPartParsed()
{
	FormPart&	currentPart = getCurrentPart();

	if (hasHeader("content-type"))
		currentPart.setContentType(getHeader("content-type"));
	if (hasHeader("content-disposition"))
		currentPart.setContentDisposition(getHeader("content-disposition"));
	m_Headers.clear();
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