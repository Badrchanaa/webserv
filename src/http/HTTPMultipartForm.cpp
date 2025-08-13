#include "HTTPMultipartForm.hpp"
#include "HTTPParser.hpp"
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
	m_Headers.clear();
}

void	HTTPMultipartForm::onNewPart()
{
	m_Parts.push_back(new FormPart());
}

FormPart*	HTTPMultipartForm::getFirstFilePart()
{
	std::vector<FormPart*>::iterator it;
	for (it = m_Parts.begin(); it != m_Parts.end(); it++)
	{
		FormPart *currPart = *it;
		HTTPHeaders::header_map_t directives = currPart->getDispositionDirectives();
		if (directives.find("filename") != directives.end())
			return currPart;
	}
	return NULL;
}

void	HTTPMultipartForm::onParseComplete()
{
	for (std::vector<FormPart *>::iterator it = m_Parts.begin(); it < m_Parts.end(); it++)
	{
		FormPart *part = *it;
		part->getBody().seal();
	}
}

FormPart&				HTTPMultipartForm::getCurrentPart()
{
	return *m_Parts.back();
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
	for (std::vector<FormPart *>::iterator it = m_Parts.begin(); it < m_Parts.end(); it++)
	{
		FormPart *part = *it;
		delete part;
	}
}