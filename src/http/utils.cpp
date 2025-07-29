#include <string>
#include "http.hpp"

header_map_t	parseHeaderDirectives(std::string headerValue, std::string::size_type startPos)
{
	header_map_t directives; 
	std::string::size_type sepPos;
	std::string::size_type pos = startPos;

	while (pos != std::string::npos)
	{
		headerValue = headerValue.substr(pos + 1, std::string::npos);
		sepPos = headerValue.find('=', 0);
		if (sepPos == std::string::npos)
			break ;
		pos = headerValue.find(';', 0);
		std::string directive = headerValue.substr(0, sepPos);
		size_t	firstNonSpace = directive.find_first_not_of(" ");
		if (firstNonSpace == std::string::npos)
			firstNonSpace = 0;
		directive = directive.substr(firstNonSpace, std::string::npos);
		directives[directive] = headerValue.substr(sepPos + 1, pos - sepPos - 1);
	}
	return directives;
}

std::string removeQuotes(std::string str)
{
    std::string::size_type posFirst = str.find_first_not_of("\"");
    std::string::size_type posLast = str.find_last_not_of("\"");

    if (posFirst == std::string::npos)
        return std::string("");

    return str.substr(posFirst, posLast - posFirst + 1);
}