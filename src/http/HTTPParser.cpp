#include "HTTPParser.hpp"
#include <iostream>
#include <string>
#include <algorithm>
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

// HTTPHeaders::header_map_t	HTTPParser::parseHeaderDirectives(std::string headerValue, std::string::size_type startPos)
// {
// 	HTTPRequest::header_map_t directives; 
// 	std::string::size_type sepPos;
// 	std::string::size_type pos = startPos;

// 	while (pos != std::string::npos)
// 	{
// 		headerValue = headerValue.substr(pos + 1, std::string::npos);
// 		sepPos = headerValue.find('=', 0);
// 		if (sepPos == std::string::npos)
// 			break ;
// 		pos = headerValue.find(';', 0);
// 		std::string directive = headerValue.substr(0, sepPos);
// 		size_t	firstNonSpace = directive.find_first_not_of(" ");
// 		if (firstNonSpace == std::string::npos)
// 			firstNonSpace = 0;
// 		directive = directive.substr(firstNonSpace, std::string::npos);
// 		directives[directive] = headerValue.substr(sepPos + 1, pos - sepPos - 1);
// 	}
// 	return directives;
// }

const char	*getStateString(HTTPParseState::state_t state);
inline bool	HTTPParser::_isCrlf(char current, char previous = LF)
{
	return ((current == CR || current == LF) && previous != current);
}

size_t	HTTPParser::_skipCrlf(HTTPParseState &parseState, const char *buff, size_t start, size_t len)
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
size_t	HTTPParser::_parseStart(HTTPRequest &request, const char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	size_t	i;

	i = _skipCrlf(parseState, buff, start, len);
	if (i == len)
		return i;
	if (parseState.getPrevChar() == CR || parseState.getReadBytes() > MAX_CRLF_BYTES)
		parseState.setState(HTTPParseState::PARSE_ERROR);
	else
		parseState.setState(HTTPParseState::PARSE_LINE_METHOD);
	return i;
}

/*
	Handle request method
*/
size_t	HTTPParser::_parseMethod(HTTPRequest &request, const char *buff, size_t start, size_t len)
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
	if (buff[i] != SP || methodSize > HTTPParser::MAX_METHOD_SIZE)
	{
		// std::cout << "method error (not SP || size > MAX_METHOD), i: " << i << std::endl;
		parseState.setState(HTTPParseState::PARSE_ERROR);
		return i;
	}
	method[methodSize] = '\0';
	request.setMethod(method);
	if (request.isError())
		return i;
	parseState.advance();
	return i + 1;
}

size_t	HTTPParser::_parseTarget(HTTPRequest &request, const char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	unsigned int	targetSize = parseState.getReadBytes();
	size_t	i = start;

	while (i < len && std::isprint(buff[i]) && targetSize < HTTPParser::MAX_REQUEST_LINE_SIZE)
	{
		if (buff[i] == SP)
			break;
		i++;
		targetSize++;
	}
	if (i == len)
	{
		request.appendUri(buff, start, i);
		parseState.setReadBytes(targetSize);
		return i;
	}
	if (buff[i] != SP || targetSize > HTTPParser::MAX_REQUEST_LINE_SIZE)
	{
		// std::cout << "target error (!SP || size > MAX_LINE) i: " << i << std::endl;
		parseState.setState(HTTPParseState::PARSE_ERROR);
		return i;
	}
	request.appendUri(buff, start, i);
	parseState.setReadBytes(0);
	if (!request.validUri())
		parseState.setState(HTTPParseState::PARSE_ERROR);
	else
		parseState.setState(HTTPParseState::PARSE_LINE_HTTP);
	return i + 1;
}

size_t	HTTPParser::_parseVersion(HTTPRequest &request, const char *buff, size_t start, size_t len)
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
			parseState.setState(HTTPParseState::PARSE_ERROR);
			return i;
		}
		count++;
		i++;
	}
	if (i < len && buff[i] == SLASH)
	{
		if (count == 4)
			parseState.advance();
		else
			parseState.setState(HTTPParseState::PARSE_ERROR);
		return i + 1;
	}
	parseState.setReadBytes(count);	
	return i;
}

size_t	HTTPParser::_parseVersionNumber(HTTPRequest &request, const char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	HTTPParseState::state_t	state = parseState.getState();
	// unsigned int	count = parseState.getReadBytes();
	size_t	i = start;
	char	expectedChar;

	while (i < len && state < HTTPParseState::PARSE_HEADER_CRLF)
	{
		switch(state)
		{
			case HTTPParseState::PARSE_LINE_VERSION_MINOR:
			case HTTPParseState::PARSE_LINE_VERSION_MAJOR:
				expectedChar = '1';
				break;
			case HTTPParseState::PARSE_LINE_VERSION_DOT:
				expectedChar = '.';
				break;
			default:
				parseState.setState(HTTPParseState::PARSE_ERROR);
				return len;
		}
		if (buff[i] != expectedChar)
		{
			std::cout << "version err (expected:" << expectedChar << ", got: " << buff[i] << ") i:" << i << std::endl;
			parseState.setState(HTTPParseState::PARSE_ERROR);
			return i;
		}
		state = parseState.advance();
		i++;
	}
	return i;
}

size_t	HTTPParser::_parseHeaderCrlf(HTTPHeaders &httpHeaders, const char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = httpHeaders.getParseState();
	size_t	i;

	i = _skipCrlf(parseState, buff, start, len);
	// std::cout << buff << std::endl;
	size_t	readBytes = parseState.getReadBytes();
	if (readBytes < 4 && i == len)
		return i;
	if (parseState.getPrevChar() == CR)
	{
		std::cout << "readBytes: " << readBytes << " i: " << i << " len: " << len << std::endl;
		// parseState.setState(HTTPParseState::PARSE_ERROR);
		parseState.setError();
		return i;
	}
	switch(readBytes)
	{
		case 2:
			parseState.setState(HTTPParseState::PARSE_HEADER_FIELD);
			break;
		case 4:
			httpHeaders.onHeadersParsed();
			// if (httpMessage.isParseComplete())
			// 	return i;
			// parseState.setState(HTTPParseState::PARSE_ERROR);
			break;
		default:
			std::cout << "err crlf header" << std::endl;
			parseState.setError();
	}
	parseState.setReadBytes(0);
	// std::cout << getStateString(parseState.getState()) << std::endl;
	return i;
}

bool HTTPParser::_notValidHeaderField(const int &c)
{
	return (c < 0 || c >= 128 || !HTTPParser::TOKEN_ALLOWED_CHARS[c]);
}

size_t	HTTPParser::_parseHeaderField(HTTPHeaders &httpHeaders, const char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = httpHeaders.getParseState();
	size_t	count = parseState.getReadBytes();
	size_t	i;
	const char *it;
	
	it = std::find_if(buff + start, buff + len, _notValidHeaderField);
	i = std::distance(buff, it);
	count += i - start;
	parseState.appendHeaderField(buff, start, i);
	if ((i < len && *it != ':') || count > HTTPParser::MAX_HEADER_SIZE)
		return (parseState.setError(), i);
	if (i == len)
		return (parseState.setReadBytes(count), i);
	parseState.setReadBytes(0);
	parseState.setState(HTTPParseState::PARSE_HEADER_VALUE);
	return i + 1;
}

bool HTTPParser::_notValidHeaderValue(const int &c)
{
	return (!std::isprint(c));
}

size_t	HTTPParser::_parseHeaderValue(HTTPHeaders &httpHeaders, const char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = httpHeaders.getParseState();
	size_t	count = parseState.getReadBytes();
	size_t	i;
	const char *it;

	it = std::find_if(buff + start, buff + len, _notValidHeaderValue);
	i = std::distance(buff, it);
	count += i - start;
	parseState.appendHeaderValue(buff, start, i);
 	if ((i < len && *it != CR) || count > HTTPParser::MAX_HEADER_SIZE)
		return (parseState.setError(), i);
	if (i == len)
		return (parseState.setReadBytes(count), i);
	httpHeaders.addHeader(parseState.getHeaderField(), parseState.getHeaderValue());
	parseState.clearHeader();
	parseState.setReadBytes(0);
	parseState.setState(HTTPParseState::PARSE_HEADER_CRLF);
	return i;
}

size_t	HTTPParser::_parseChunk(HTTPRequest &request, const char *buff, size_t start, size_t len)
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
				parseState.setState(HTTPParseState::PARSE_DONE);
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

size_t	HTTPParser::_parseChunkData(HTTPRequest &request, const char *buff, size_t start, size_t len)
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
	if (chunkSize == 0) // end chunk
	{
		if (buff[start] != CR)
			return (parseState.setError(), start);
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

size_t	_parseMultipartBoundarySuffix(HTTPRequest &request, const char *buff, size_t start, size_t len)
{
	HTTPParseState&	parseState = request.multipartForm->getParseState();
	size_t readBytes = parseState.getReadBytes();
	const char *BOUNDARY_DELIMITER = "--";
	const char *BOUNDARY_CRLF = "\r\n";
	size_t	bytesToRead = 2 - readBytes;

	if (len - start == 1)
	{
		if (buff[start] != CR && buff[start] != '-')
			return parseState.setError(), len;
		parseState.setReadBytes(1);
		parseState.setPrevChar(buff[start]);
		return (start + 1);
	}
	if (!memcmp(BOUNDARY_DELIMITER + readBytes, buff + start, bytesToRead))
	{
		parseState.setState(HTTPParseState::PARSE_DONE);
		request.multipartForm->onParseComplete();
	}
	else if (!memcmp(BOUNDARY_CRLF + readBytes, buff + start, bytesToRead))
	{
		// new form part
		request.multipartForm->onNewPart();
		parseState.setPrevChar(LF);
		parseState.setState(HTTPParseState::PARSE_HEADER_FIELD);
	}
	else
		parseState.setError();

	return (start +  bytesToRead);
}

size_t	_parseMultipartBoundary(HTTPRequest &request, const char *buff, size_t start, size_t len)
{
	HTTPParseState&	parseState = request.multipartForm->getParseState();
	size_t readBytes = parseState.getReadBytes();
	std::string boundary;

	if (request.multipartForm->getPartsCount() > 0)
		boundary = "\r\n--" + request.multipartForm->getBoundary();
	else
		boundary = "--" + request.multipartForm->getBoundary();
	size_t	remainingBoundaryBytes = boundary.length() - readBytes;
	size_t	availableBytes = (len - start);
	size_t	bytesToRead = std::min(availableBytes, remainingBoundaryBytes);

	size_t i = 0;
	while (i < bytesToRead && boundary[readBytes + i] == buff[start + i])
		i++;

	if (i < bytesToRead) // mismatch
	{
		request.multipartForm->getCurrentPart().getBody().append(boundary.c_str(), readBytes + i);
		parseState.setReadBytes(0);
		parseState.setState(HTTPParseState::PARSE_BODY);
		return (start + i);
	}

	if (availableBytes < remainingBoundaryBytes) //	buffer end
	{
		parseState.setReadBytes(readBytes + i);
		return (start + i);
	}
	// boundary match
	parseState.setReadBytes(0);
	parseState.setState(HTTPParseState::PARSE_MULTIPART_BOUNDARY_SUFFIX);
	return (start + i);
}

size_t	HTTPParser::_parseMultipartBody(HTTPRequest &request, const char *buff, size_t start, size_t len)
{
	HTTPParseState&	parseState = request.multipartForm->getParseState();
	FormPart& formPart = request.multipartForm->getCurrentPart();

	size_t	i = start;	
	while (i < len && buff[i] != CR)
		i++;
	formPart.getBody().append(buff + start, i - start);
	if (i < len)
	{
		// end part body start boundary
		parseState.setState(HTTPParseState::PARSE_MULTIPART_BOUNDARY);
		return i;
	}
	return (i);
}

size_t	HTTPParser::_parseMultipartForm(HTTPRequest &request, const char *buff, size_t start, size_t len)
{
	HTTPMultipartForm *form = request.multipartForm;
	HTTPParseState&	parseState = form->getParseState();

	size_t	i;
	// std::cout << "parse multipart start = " << start  << "->" << (int)buff[start] << std::endl;
	std::cout << "parse multipart state = " << getStateString(parseState.getState()) << std::endl;
	switch(parseState.getState())
	{
		case HTTPParseState::PARSE_MULTIPART_BOUNDARY:
			i = _parseMultipartBoundary(request, buff, start, len);
			break;
		case HTTPParseState::PARSE_MULTIPART_BOUNDARY_SUFFIX:
			i = _parseMultipartBoundarySuffix(request, buff, start, len);
			break;
		case HTTPParseState::PARSE_HEADER_FIELD:
			i = _parseHeaderField(*form, buff, start, len);
			break;
		case HTTPParseState::PARSE_HEADER_VALUE:
			i = _parseHeaderValue(*form, buff, start, len);
			break;
		case HTTPParseState::PARSE_HEADER_CRLF:
			i = _parseHeaderCrlf(*form, buff, start, len);
			break;
		case HTTPParseState::PARSE_BODY:
			i = _parseMultipartBody(request, buff, start, len);
			break;
		default:
			request.getParseState().setState(parseState.getState());
			return len;
	}
	return i;
}

size_t	HTTPParser::_parseRawBody(HTTPRequest &request, const char *buff, size_t start, size_t len)
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
		// parseState.setState(HTTPParseState::PARSE_DONE);
		request.onBodyDone();
		std::cout << "body done" << std::endl;
	}
	else
		parseState.setReadBytes(count);
	return start + bytesToRead;
}

size_t	HTTPParser::_parseBody(HTTPRequest &request, const char *buff, size_t start, size_t len)
{
	std::cout << "PARSE BODY" << std::endl;
	if (request.isTransferChunked())
		return _parseChunk(request, buff, start, len);

	if (request.isMultipartForm())
		return _parseMultipartForm(request, buff, start, len);
	
	return _parseRawBody(request, buff, start, len);
}

size_t	HTTPParser::_parseCgiBody(HTTPMessage &httpMessage, const char *buff, size_t start, size_t len)
{
	HTTPParseState	&parseState = httpMessage.getParseState();
	size_t			count = parseState.getReadBytes();
	size_t			bytesToRead;
	size_t			contentLength;
	
	if (httpMessage.hasHeader("content-length"))
		contentLength = httpMessage.getContentLength();
	else
		contentLength = SIZE_MAX;
	bytesToRead = std::min(contentLength - count, len - start); //std::min(len, remaining);
	count += bytesToRead;
	httpMessage.appendBody(buff + start, bytesToRead);	
	if (count == contentLength)
	{
		httpMessage.onBodyDone();
		std::cout << "body done" << std::endl;
	}
	else
		parseState.setReadBytes(count);
	return start + bytesToRead;
}

void	HTTPParser::parseRequest(HTTPRequest &request, const char *buff, size_t len)
{
	HTTPParseState	&parseState = request.getParseState();
	HTTPParseState::state_t	state;
	unsigned int	offset;

	offset = 0;
	while (offset < len)
	{
		state = parseState.getState();
		std::cout << "parse state: " << getStateString(state) << std::endl;
		switch(state)
		{
			case HTTPParseState::PARSE_LINE_START:
				offset = _parseStart(request, buff, offset, len);
				break;
			case HTTPParseState::PARSE_LINE_METHOD:
				offset = _parseMethod(request, buff, offset, len);
				break;
			case HTTPParseState::PARSE_LINE_TARGET:
				offset = _parseTarget(request, buff, offset, len);
				break;
			case HTTPParseState::PARSE_LINE_HTTP:
				offset = _parseVersion(request, buff, offset, len);
				break;
			case HTTPParseState::PARSE_LINE_VERSION_MINOR:
			case HTTPParseState::PARSE_LINE_VERSION_DOT:
			case HTTPParseState::PARSE_LINE_VERSION_MAJOR:
				offset = _parseVersionNumber(request, buff, offset, len);
				break;
			case HTTPParseState::PARSE_HEADER_CRLF:
				offset = _parseHeaderCrlf(request, buff, offset, len);
				break;
			case HTTPParseState::PARSE_HEADER_FIELD:
				offset = _parseHeaderField(request, buff, offset, len);
				break;
			case HTTPParseState::PARSE_HEADER_VALUE:
				offset = _parseHeaderValue(request, buff, offset, len);
				break;
			case HTTPParseState::PARSE_BODY:
				offset = _parseBody(request, buff, offset, len);
				break;
			default:
				return;
		}
	}
}


void	HTTPParser::_parseCgiError(HTTPResponse &response, const char *buff, size_t len)
{
	(void) response;
	(void) buff;
	(void) len;
	// (void) ;

	// std::string warningMessage = " Warning";
	// std::string errorMessage = " Error";

}

void	HTTPParser::parseCgi(HTTPResponse &response, const char *buff, size_t len)
{
	HTTPParseState	&parseState = response.getParseState();
	HTTPParseState::state_t	state;
	unsigned int	offset;

	offset = 0;
	
	while (offset < len)
	{
		state = parseState.getState();
		std::cout << "parse state: " << getStateString(state) << std::endl;
		switch(state)
		{
			case HTTPParseState::PARSE_HEADER_CRLF:
				offset = _parseHeaderCrlf(response, buff, offset, len);
				break;
			case HTTPParseState::PARSE_HEADER_FIELD:
				offset = _parseHeaderField(response, buff, offset, len);
				break;
			case HTTPParseState::PARSE_HEADER_VALUE:
				offset = _parseHeaderValue(response, buff, offset, len);
				break;
			case HTTPParseState::PARSE_BODY:
			case HTTPParseState::PARSE_ERROR:
				offset = _parseCgiBody(response, buff, offset, len);
				break;
			default:
				std::cerr << "CGI PARSER: UNEXPECTED PARSE STATE: " << getStateString(state) << std::endl;
				return;
		}
	}
}

const char	*getStateString(HTTPParseState::state_t state)
{
	switch(state)
	{
			case HTTPParseState::PARSE_LINE_START:
				return "PARSE_LINE_START";
			case HTTPParseState::PARSE_MULTIPART_BOUNDARY:
				return "PARSE_MULTIPART_BOUNDARY";
			case HTTPParseState::PARSE_MULTIPART_BOUNDARY_SUFFIX:
				return "PARSE_MULTIPART_BOUNDARY_SUFFIX";
			case HTTPParseState::PARSE_LINE_METHOD:
				return "PARSE_LINE_METHOD";
			case HTTPParseState::PARSE_LINE_TARGET:
				return "PARSE_LINE_TARGET";
			case HTTPParseState::PARSE_LINE_HTTP:
				return "PARSE_LINE_HTTP";
			case HTTPParseState::PARSE_LINE_VERSION_MINOR:
			case HTTPParseState::PARSE_LINE_VERSION_DOT:
			case HTTPParseState::PARSE_LINE_VERSION_MAJOR:
				return "PARSE_LINE_VERSION";
			case HTTPParseState::PARSE_HEADER_CRLF:
				return "PARSE_HEADER_CRLF";
			case HTTPParseState::PARSE_HEADER_FIELD:
				return "PARSE_HEADER_FIELD";
			case HTTPParseState::PARSE_HEADER_VALUE:
				return "PARSE_HEADER_VALUE";
			case HTTPParseState::PARSE_BODY:
				return "PARSE_BODY";
			case HTTPParseState::PARSE_DONE:
				return "PARSE_DONE";
			case HTTPParseState::PARSE_ERROR:
				return "PARSE_ERROR";
			default:
				return "UNKNOWN_STATE";
	}
}

HTTPParser::HTTPParser(void)
{
	
}