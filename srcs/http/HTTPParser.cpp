#include "HTTPParser.hpp"
#include <iostream>
#include <string>

const	unsigned int HTTPParser::MAX_HEADER_SIZE = 8192;
const	unsigned int HTTPParser::MAX_REQUEST_LINE_SIZE = 4096;
const	unsigned int HTTPParser::MAX_METHOD_SIZE = 10;
const	unsigned int HTTPParser::MAX_BODY_SIZE = 1024 * 1024;

const uint8_t HTTPParser::TOKEN_ALLOWED_CHARS[128] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-15
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16-31
			0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, // 32-47 (!, #, $, %, &, ', *, +, -, .)
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 48-63 (0 - 9)
			
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 64-79 (A-O)
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, // 80-95 (P-Z, ^, _)

			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 96-111 (a-o, `)
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, // 112-127 (p-z, |, ~)
};

inline bool	HTTPParser::_isCrlf(char current, char previous = LF)
{
	return ((current == CR || current == LF) && previous != current);
}

size_t	HTTPParser::_skipCrlf(HTTPParseState &parseState, char *buff, size_t start, size_t len) const
{
	size_t	i;
	unsigned int count;
	char prev;

	i = start;
	prev = parseState.getPrevChar();
	count = parseState.getReadBytes();
	while (i < len && HTTPParser::_isCrlf(buff[i], prev))
	{
		if (count > MAX_CRLF_BYTES)
			break;
		count++;
		prev = buff[i++];
	}
	parseState.setReadBytes(count);
	parseState.setPrevChar(prev);
	return i;
}

/*
	Handles CRLF before Request-Line.
*/
size_t	HTTPParser::_parseStart(HTTPRequest &request, char *buff, size_t start, size_t len) const
{
	HTTPParseState	&parseState = request.getParseState();
	size_t	i;

	i = _skipCrlf(parseState, buff, start, len);
	if (i == len)
		return i;

	if (parseState.getPrevChar() == CR || parseState.getReadBytes() > MAX_CRLF_BYTES)
		parseState.setState(HTTPParseState::REQ_ERROR);
	else
		parseState.setState(HTTPParseState::REQ_LINE_METHOD);
	return i;
}

/*
	Handle request method
*/
size_t	HTTPParser::_parseMethod(HTTPRequest &request, char *buff, size_t start, size_t len) const
{
	HTTPParseState	&parseState = request.getParseState();
	size_t	i = start;
	char	*method = parseState.getMethod();	
	unsigned int	methodSize = parseState.getReadBytes();

	while (i < len && std::isalpha(buff[i]) && methodSize <= HTTPParser::MAX_METHOD_SIZE)
	{
		method[methodSize++] = buff[i];
		i++;
	}
	parseState.setReadBytes(methodSize);
	if (i == len)
		return i;
	// TODO: add constant for max method size
	if (buff[i] != SP || methodSize > HTTPParser::MAX_METHOD_SIZE)
	{
		// std::cout << "method error (not SP || size > MAX_METHOD), i: " << i << std::endl;
		parseState.setState(HTTPParseState::REQ_ERROR);
		return i;
	}
	method[methodSize] = '\0';
	request.setMethod(method);
	parseState.advance();
	return i + 1;
}

size_t	HTTPParser::_parseTarget(HTTPRequest &request, char *buff, size_t start, size_t len) const
{
	HTTPParseState	&parseState = request.getParseState();
	unsigned int	targetSize = parseState.getReadBytes();
	size_t	i = start;

	// TODO: add constant for max target size
	while (i < len && std::isprint(buff[i]) && targetSize < )
	{
		if (buff[i] == SP)
			break;
		i++;
		targetSize++;
	}
	if (i == len)
	{
		request.appendToPath(buff, start, len);
		parseState.setReadBytes(targetSize);
		return i;
	}
	if (buff[i] != SP || targetSize > HTTPParser::MAX_REQUEST_LINE_SIZE)
	{
		// std::cout << "target error (!SP || size > MAX_LINE) i: " << i << std::endl;
		parseState.setState(HTTPParseState::REQ_ERROR);
		return i;
	}
	request.appendToPath(buff, start, i);
	parseState.setReadBytes(0);
	if (!request.validPath())
		parseState.setState(HTTPParseState::REQ_ERROR);
	else
		parseState.setState(HTTPParseState::REQ_LINE_HTTP);
	return i + 1;
}

size_t	HTTPParser::_parseVersion(HTTPRequest &request, char *buff, size_t start, size_t len) const
{
	HTTPParseState	&parseState = request.getParseState();
	unsigned int	count = parseState.getReadBytes();
	size_t	i = start;

	while (i < len)
	{
		if (buff[i] == SLASH)
			break;
		if (count > 3 || buff[i] != HTTP[count])
		{
			parseState.setState(HTTPParseState::REQ_ERROR);
			return i;
		}
		count++;
		i++;
	}
	if (buff[i] == SLASH)
	{
		if (count == 4)
			parseState.advance();
		else
			parseState.setState(HTTPParseState::REQ_ERROR);
		return i + 1;
	}
	parseState.setReadBytes(count);	
	return i;
}

size_t	HTTPParser::_parseVersionNumber(HTTPRequest &request, char *buff, size_t start, size_t len) const
{
	HTTPParseState	&parseState = request.getParseState();
	HTTPParseState::requestState	state = parseState.getState();
	// unsigned int	count = parseState.getReadBytes();
	size_t	i = start;
	char	expectedChar;

	while (i < len && state < HTTPParseState::REQ_HEADER_CRLF)
	{
		switch(state)
		{
			case HTTPParseState::REQ_LINE_VERSION_MINOR:
			case HTTPParseState::REQ_LINE_VERSION_MAJOR:
				expectedChar = '1';
				break;
			case HTTPParseState::REQ_LINE_VERSION_DOT:
				expectedChar = '.';
				break;
			default:
				parseState.setState(HTTPParseState::REQ_ERROR);
				return len;
		}
		if (buff[i] != expectedChar)
		{
			std::cout << "version err (expected:" << expectedChar << ", got: " << buff[i] << ") i:" << i << std::endl;
			parseState.setState(HTTPParseState::REQ_ERROR);
			return i;
		}
		state = parseState.advance();
		i++;
	}
	return i;
}

size_t	HTTPParser::_parseHeaderCrlf(HTTPRequest &request, char *buff, size_t start, size_t len) const
{
	HTTPParseState	&parseState = request.getParseState();
	size_t	i;

	i = _skipCrlf(parseState, buff, start, len);
	if (i == len)
		return i;
	size_t	readBytes = parseState.getReadBytes();
	if (readBytes == 1 || parseState.getPrevChar() == CR)
	{
		parseState.setState(HTTPParseState::REQ_ERROR);
		return i;
	}
	switch(readBytes >> 1)
	{
		case 1:
			parseState.setState(HTTPParseState::REQ_HEADER_FIELD);
			break;
		case 2:
			parseState.setState(HTTPParseState::REQ_BODY);
			break;
		default:
			parseState.setState(HTTPParseState::REQ_ERROR);
	}
	parseState.setReadBytes(0);
	return i;
}

size_t	HTTPParser::_parseHeaderField(HTTPRequest &request, char *buff, size_t start, size_t len) const
{
	HTTPParseState	&parseState = request.getParseState();
	size_t	i;
	size_t	count = parseState.getReadBytes();
	bool	isAllowed;
	char	c;
	bool	isErr = false;
	
	i = start;
	while (i < len)
	{
		if (count > HTTPParser::MAX_HEADER_SIZE)
		{
			parseState.setState(HTTPParseState::REQ_ERROR);
			break;
		}
		c = buff[i];
		if (c == ':')
			break;
		isAllowed = c >= 0 && c < 128 && HTTPParser::TOKEN_ALLOWED_CHARS[c];
		if (!isAllowed)
		{
			parseState.setState(HTTPParseState::REQ_ERROR);
			return i;
		}
		i++;
		count++;
	}
	if (i == len)
		return i;
	parseState.advance();
	return i;
}


void	HTTPParser::parse(HTTPRequest &request, char *buff, size_t len) const
{
	HTTPParseState	&parseState = request.getParseState();
	HTTPParseState::requestState	state;
	unsigned int	offset;

	offset = 0;
	while (offset < len)
	{
		state = parseState.getState();
		std::cout << "parse state: " << state << std::endl;
		switch(state)
		{
			case HTTPParseState::REQ_LINE_START:
				offset = _parseStart(request, buff, offset, len);
				break;
			case HTTPParseState::REQ_LINE_METHOD:
				offset = _parseMethod(request, buff, offset, len);
				break;
			case HTTPParseState::REQ_LINE_TARGET:
				offset = _parseTarget(request, buff, offset, len);
				break;
			case HTTPParseState::REQ_LINE_HTTP:
				offset = _parseVersion(request, buff, offset, len);
				break;
			case HTTPParseState::REQ_LINE_VERSION_MINOR:
			case HTTPParseState::REQ_LINE_VERSION_DOT:
			case HTTPParseState::REQ_LINE_VERSION_MAJOR:
				offset = _parseVersionNumber(request, buff, offset, len);
				break;
			default:
				return;
		}
	}
}

HTTPParser::HTTPParser(void)
{
	
}

HTTPParser::HTTPParser(const HTTPParser &other)
{
	(void)other;
}

HTTPParser& HTTPParser::operator=(const HTTPParser &other)
{
	(void)other;
	return *this;
}

HTTPParser::~HTTPParser(void)
{
	
}