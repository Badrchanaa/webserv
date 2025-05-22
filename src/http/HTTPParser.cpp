#include "HTTPParser.hpp"
#include <iostream>
#include <string>
#include <exception>

#ifdef DEBUG
 #include <cassert>
#endif

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

const char	*getStateString(HTTPParseState::requestState state);
inline bool	HTTPParser::_isCrlf(char current, char previous = LF)
{
	return ((current == CR || current == LF) && previous != current);
}

size_t	HTTPParser::_skipCrlf(HTTPParseState &parseState, char *buff, size_t start, size_t len)
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
size_t	HTTPParser::_parseStart(HTTPRequest &request, char *buff, size_t start, size_t len)
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
size_t	HTTPParser::_parseMethod(HTTPRequest &request, char *buff, size_t start, size_t len)
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
	if (request.isError())
		return i;
	parseState.advance();
	return i + 1;
}

size_t	HTTPParser::_parseTarget(HTTPRequest &request, char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	unsigned int	targetSize = parseState.getReadBytes();
	size_t	i = start;

	// TODO: add constant for max target size
	while (i < len && std::isprint(buff[i]) && targetSize < HTTPParser::MAX_REQUEST_LINE_SIZE)
	{
		if (buff[i] == SP)
			break;
		i++;
		targetSize++;
	}
	if (i == len)
	{
		request.appendToPath(buff, start, i);
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

size_t	HTTPParser::_parseVersion(HTTPRequest &request, char *buff, size_t start, size_t len)
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

size_t	HTTPParser::_parseVersionNumber(HTTPRequest &request, char *buff, size_t start, size_t len)
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

size_t	HTTPParser::_parseHeaderCrlf(HTTPRequest &request, char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	size_t	i;

	i = _skipCrlf(parseState, buff, start, len);
	// std::cout << buff << std::endl;
	size_t	readBytes = parseState.getReadBytes();
	if (readBytes < 4 && i == len)
		return i;
	if (parseState.getPrevChar() == CR)
	{
		parseState.setState(HTTPParseState::REQ_ERROR);
		std::cout << "header crlf error, readbytes: " << readBytes << " prev: " << parseState.getPrevChar() << std::endl;
		return i;
	}
	switch(readBytes)
	{
		case 2:
			parseState.setState(HTTPParseState::REQ_HEADER_FIELD);
			break;
		case 4:
			request.processHeaders();
			if (request.isComplete())
				return i;
			parseState.setState(HTTPParseState::REQ_BODY);
			// parseState.setState(HTTPParseState::REQ_ERROR);
			break;
		default:
			std::cout << "err crlf header" << std::endl;
			parseState.setState(HTTPParseState::REQ_ERROR);
	}
	parseState.setReadBytes(0);
	// std::cout << getStateString(parseState.getState()) << std::endl;
	return i;
}

size_t	HTTPParser::_parseHeaderField(HTTPRequest &request, char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	size_t	count = parseState.getReadBytes();
	size_t	i;
	int		c;
	
	i = start;
	while (i < len)
	{
		if (count > HTTPParser::MAX_HEADER_SIZE)
		{
			parseState.setState(HTTPParseState::REQ_ERROR);
			return i;
		}
		c = static_cast<int>(buff[i]);
		if (c == ':')
			break;
		if (c < 0 || c >= 128 || !HTTPParser::TOKEN_ALLOWED_CHARS[c])
		{
			parseState.setState(HTTPParseState::REQ_ERROR);
			return i;
		}
		else if (std::isalpha(c))
			buff[i] = std::tolower(c);
		i++;
		count++;
	}
	parseState.setReadBytes(count);
	parseState.appendHeaderField(buff, start, i);
	if (i == len)
		return i;
	parseState.setReadBytes(0);
	parseState.setState(HTTPParseState::REQ_HEADER_VALUE);
	return i + 1;
}

size_t	HTTPParser::_parseHeaderValue(HTTPRequest &request, char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	size_t	count = parseState.getReadBytes();
	size_t	i;

	i = start;
	while (i < len)
	{
		if (buff[i] == CR)
			break;
		else if(!std::isprint(buff[i]) || count > HTTPParser::MAX_HEADER_SIZE)
		{
			parseState.setState(HTTPParseState::REQ_ERROR);
			return i;
		}
		i++;
		count++;
	}
	parseState.appendHeaderValue(buff, start, i);
	parseState.setReadBytes(count);
	if (i == len)
		return i;
	request.addHeader(parseState.getHeaderField(), parseState.getHeaderValue());
	parseState.clearHeader();
	parseState.setReadBytes(0);
	parseState.setState(HTTPParseState::REQ_HEADER_CRLF);
	return i;
}

size_t	HTTPParser::_parseChunk(HTTPRequest &request, char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	size_t			count = parseState.getReadBytes();
	size_t			i;
	HTTPParseState::chunkState state;
	char			c;

	state = parseState.getChunkState();
	// std::cout << "enter state: " << state << std::endl;
	for (i = start; i < len; i++)
	{
		std::cout << "loop i " << i << " char : " << buff[i] << std::endl;
		if (count + (i - start) > HTTPParser::MAX_BODY_SIZE)
			return (parseState.setError(), i);
		c = buff[i];
		switch (state)
		{
			case HTTPParseState::CHUNK_SIZE:
				if (std::isxdigit(c))
					parseState.appendChunkSize(c);
				else if (c == ';') // chunk extension delimiter
					state = HTTPParseState::CHUNK_EXT;
				else if (c == CR)
					state = HTTPParseState::CHUNK_CRLF;
				else
					return (parseState.setError(), i);
				break;
			case HTTPParseState::CHUNK_EXT:
				if (c == CR)
					state = HTTPParseState::CHUNK_CRLF;
				// skip chunk extension
				break;
			case HTTPParseState::CHUNK_CRLF:
				if (c == LF)
				{
					std::cout << "IN CHUNK CRLF i: " << i << std::endl;
					if ( not parseState.validateChunkSize())
						return (parseState.setError(), i);
					state = HTTPParseState::CHUNK_DATA;
					// parseState.setchunkPos(0);
				}
				else
					return (parseState.setError(), i);
				break;
			case HTTPParseState::CHUNK_DATA:
				std::cout << "PARSE CHUNK DATA SIZE: " << parseState.getChunkSize() << std::endl;
				i = _parseChunkData(request, buff, i, len);
				if (i == len)
					break;
				std::cout << "AFTER CHUNK DATA i: " << i << std::endl;
				
				if (parseState.getChunkSize() == 0)
					state = HTTPParseState::CHUNK_END;
				else if (parseState.getChunkSize() ==parseState.getChunkPos())
				{
					std::cout << "CHUNK COMPLETE" << std::endl;
					if (buff[i] != CR)
						return (parseState.setError(), i);
					state = HTTPParseState::CHUNK_DATA_CRLF;
				}
				// else ??
				break;
			case HTTPParseState::CHUNK_DATA_CRLF:
				std::cout  <<  "CHUNK DATA CRLF" << std::endl;
				if (c == LF)
					state = HTTPParseState::CHUNK_SIZE;
				else
					return (parseState.setError(), i);
				parseState.resetChunk();
				break;
			case HTTPParseState::CHUNK_END:
				std::cout << "REQUEST DONEE CHUNK" << std::endl;
				if (c != LF)
					return (parseState.setError(), i);
				parseState.setState(HTTPParseState::REQ_DONE);
				std::cout << "REQUEST DONEE CHUNK" << std::endl;
				return i;
			default:
				std::cout << "Unexpected Chunk State" << std::endl;
				return (parseState.setError(), i);
		}
	}
	// std::cout << "leave state: " << state << std::endl;
	parseState.setChunkState(state);
	parseState.setReadBytes(count + i - start);
	return i;
}

size_t	HTTPParser::_parseChunkData(HTTPRequest &request, char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	size_t			chunkSize;
	size_t			chunkPos;
	size_t			remainingBytes;
	size_t			bytesToRead;

	chunkPos = parseState.getChunkPos();
	chunkSize = parseState.getChunkSize();
	remainingBytes = chunkSize - chunkPos;

	if (remainingBytes == 0)
		return start;
	#ifdef DEBUG
	assert(remainingBytes >= 0);
	#endif

	if (chunkSize == 0) // end chunk
	{
		if (buff[start] != CR)
		{
			std::cout << "is not cr" <<std::endl;
			return (parseState.setError(), start);
		}
		return start;
	}
	// if (count + remainingBytes > HTTPParser::MAX_BODY_SIZE)
	// 	return (parseState.setError(), start);
	if (start == len)
		return start;
	bytesToRead = std::min(len - start, remainingBytes);
	if (request.isMultipartForm())
		return _parseMultipartForm(request, buff, start, start + bytesToRead);
	if (!request.appendBody(buff + start, bytesToRead))
		parseState.setError();

	parseState.incrementChunkPos(bytesToRead);
	return start + bytesToRead;
}

size_t	HTTPParser::_parseMultipartForm(HTTPRequest &request, char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	(void) request;
	(void) buff;
	(void) start;
	parseState.setState(HTTPParseState::REQ_DONE);
	return len;
}

size_t	HTTPParser::_parseRawBody(HTTPRequest &request, char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	size_t			count = parseState.getReadBytes();
	size_t			bytesToRead;
	// size_t			remaining;
	size_t			contentLength = request.getContentLength();

	bytesToRead = std::min(contentLength - count, len - start); //std::min(len, remaining);
	count += bytesToRead;
	request.appendBody(buff + start, bytesToRead);	
	
	if (count == contentLength)
	{
		parseState.setState(HTTPParseState::REQ_DONE);
		std::cout << "body done" << std::endl;
	}
	else
		parseState.setReadBytes(count);
	return start + bytesToRead;
}

size_t	HTTPParser::_parseBody(HTTPRequest &request, char *buff, size_t start, size_t len)
{
	std::cout << "IN PARSE BODY" << std::endl;
	if (request.isTransferChunked())
		return _parseChunk(request, buff, start, len);
	if (request.isMultipartForm())
		return _parseMultipartForm(request, buff, start, len);
	
	return _parseRawBody(request, buff, start, len);
}

void	HTTPParser::parse(HTTPRequest &request, char *buff, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	HTTPParseState::requestState	state;
	unsigned int	offset;

	offset = 0;
	while (offset < len)
	{
		state = parseState.getState();
		std::cout << "parse state: " << getStateString(state) << std::endl;
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
			case HTTPParseState::REQ_HEADER_CRLF:
				offset = _parseHeaderCrlf(request, buff, offset, len);
				break;
			case HTTPParseState::REQ_HEADER_FIELD:
				offset = _parseHeaderField(request, buff, offset, len);
				break;
			case HTTPParseState::REQ_HEADER_VALUE:
				offset = _parseHeaderValue(request, buff, offset, len);
				break;
			case HTTPParseState::REQ_BODY:
				offset = _parseBody(request, buff, offset, len);
				break;
			default:
				return;
		}
	}
}

const char	*getStateString(HTTPParseState::requestState state)
{
	switch(state)
	{
			case HTTPParseState::REQ_LINE_START:
				return "REQ_LINE_START";
			case HTTPParseState::REQ_LINE_METHOD:
				return "REQ_LINE_METHOD";
			case HTTPParseState::REQ_LINE_TARGET:
				return "REQ_LINE_TARGET";
			case HTTPParseState::REQ_LINE_HTTP:
				return "REQ_LINE_HTTP";
			case HTTPParseState::REQ_LINE_VERSION_MINOR:
			case HTTPParseState::REQ_LINE_VERSION_DOT:
			case HTTPParseState::REQ_LINE_VERSION_MAJOR:
				return "REQ_LINE_VERSION";
			case HTTPParseState::REQ_HEADER_CRLF:
				return "REQ_HEADER_CRLF";
			case HTTPParseState::REQ_HEADER_FIELD:
				return "REQ_HEADER_FIELD";
			case HTTPParseState::REQ_HEADER_VALUE:
				return "REQ_HEADER_VALUE";
			case HTTPParseState::REQ_BODY:
				return "REQ_BODY";
			case HTTPParseState::REQ_DONE:
				return "REQ_DONE";
			case HTTPParseState::REQ_ERROR:
				return "REQ_ERROR";
			default:
				return "UNKNOWN_STATE";
	}
}

HTTPParser::HTTPParser(void)
{
	
}