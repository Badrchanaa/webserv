#include "HTTPParser.hpp"
#include "HTTPRequest.hpp"
#include <unistd.h>
#include <iostream>

#define BUFFER_SIZE 64

int main()
{
	char buff[BUFFER_SIZE];
	int rbytes;

	rbytes = read(0, buff, BUFFER_SIZE);
	HTTPRequest request;
	while (rbytes > 0)
	{
		HTTPParser::parse(request, buff, rbytes);
		if (request.isComplete())
			break;
		rbytes = read(0, buff, BUFFER_SIZE);
	}
	HTTPParseState &parseState = request.getParseState();
	HTTPParseState::requestState state = parseState.getState();
	if (state == HTTPParseState::REQ_ERROR)
	{
		std::cout << "bad request" << std::endl;
		return 0;
	}
	else
		std::cout << "request parsed successfully" << std::endl;
	std::cout << "request.path: '" << request.getPath() << "'" << std::endl;
	HTTPRequest::HeaderMap headers = request.getHeaders();
	for (HTTPRequest::HeaderMap::iterator it = headers.begin(); it != headers.end(); it++)
	{
		std::cout << "request." << it->first << " = '" << it->second << "'" << std::endl;
	}
	return 0;
}