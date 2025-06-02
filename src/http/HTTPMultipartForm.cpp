#include "HTTPMultipartForm.hpp"
#include <iostream>
#include <string>

HTTPMultipartForm::HTTPMultipartForm(HTTPMessage::header_map_t &mediaTypes)
{
	m_Boundary = mediaTypes["boundary"];
}

HTTPMultipartForm::HTTPMultipartForm(const HTTPMultipartForm &other)
{
	*this = other;	
}

void	HTTPMultipartForm::addPart(FormPart& part)
{
	m_Parts.push_back(part);
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