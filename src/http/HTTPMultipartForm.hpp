#ifndef __HTTPMULTIPARTFORM_HPP__
# define __HTTPMULTIPARTFORM_HPP__

#include <string>

typedef enum
{
	FORM_HEADER_FIELD,
	FORM_HEADER_VALUE,
	FORM_BODY,
} multipartFormState;

class HTTPMultipartForm
{
	public:
		HTTPMultipartForm(void);
		HTTPMultipartForm(const HTTPMultipartForm &other);
		HTTPMultipartForm& operator=(const HTTPMultipartForm &other);
		~HTTPMultipartForm();
	private:
		multipartFormState	formState;
		
};

#endif