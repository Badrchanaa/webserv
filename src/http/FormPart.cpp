#include "FormPart.hpp"
#include "http.hpp"

FormPart::FormPart(void)
{
}

FormPart::FormPart(const FormPart &part)
{
    std::cout << "form part cp" << std::endl;
    (void)part;
    return;
}

FormPart::~FormPart()
{

}

void	FormPart::setContentType(const std::string &contentType)

{
    m_ContentType = contentType;
}

void	FormPart::setContentDisposition(const std::string &contentDisposition)
{
    m_ContentDisposition = contentDisposition;
}

HTTPHeaders::header_map_t	FormPart::getDispositionDirectives()
{
    HTTPHeaders::header_map_t directives;
    std::string::size_type pos = m_ContentDisposition.find(";");
    if (pos == std::string::npos)
        return directives;
    return parseHeaderDirectives(
        m_ContentDisposition,
        pos	
    );
}

std::string	FormPart::getContentDisposition()
{
    return m_ContentDisposition;
}

std::string	FormPart::getContentType()
{
    return m_ContentType;
}

HTTPBody&	FormPart::getBody()
{
    return m_Body;
}