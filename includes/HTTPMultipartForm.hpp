#ifndef __HTTPMULTIPARTFORM_HPP__
# define __HTTPMULTIPARTFORM_HPP__

#include <string>

class HTTPMultipartForm
{
	public:
		HTTPMultipartForm(void);
		HTTPMultipartForm(const HTTPMultipartForm &other);
		HTTPMultipartForm& operator=(const HTTPMultipartForm &other);
		~HTTPMultipartForm();
	private:
		
};

#endif