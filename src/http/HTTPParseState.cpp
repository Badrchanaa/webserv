#include "HTTPParseState.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include "http.hpp"

HTTPParseState::HTTPParseState(void): m_PrevChar(LF), m_State(PARSE_LINE_START), m_ReadBytes(0), m_ChunkSizeStr(), m_chunkPos(0), m_ChunkState(CHUNK_SIZE)
{
}

HTTPParseState::HTTPParseState(const HTTPParseState &other)
{
	m_ReadBytes = other.m_ReadBytes;
	m_State = other.m_State;
}

HTTPParseState& HTTPParseState::operator=(const HTTPParseState &other)
{
	m_ReadBytes = other.m_ReadBytes;
	m_State = other.m_State;
	return *this;
}

bool	HTTPParseState::isComplete() const
{
	return m_State == PARSE_DONE || m_State == PARSE_ERROR;
}

bool	HTTPParseState::isError() const
{
	return m_State == PARSE_ERROR;
}

HTTPParseState::state_t	HTTPParseState::getState() const
{
	return m_State;
}

std::string HTTPParseState::getHeaderField() const
{
	return m_HeaderField;
}

std::string HTTPParseState::getHeaderValue() const
{
	return m_HeaderValue;
}

void	HTTPParseState::appendHeaderValue(const char *buff, size_t start, size_t end)
{
	if (start == end)
		return;
	for (size_t i = start; i < end; i++)
	{
		m_HeaderValue += buff[i];
	}
	// m_HeaderValue.append(buff, start, end - start);
}

void	HTTPParseState::appendHeaderField(const char *buff, size_t start, size_t end)
{
	if (start == end)
		return;
	for (size_t i = start; i < end; i++)
	{
		m_HeaderField += buff[i];
	}
	// m_HeaderField.append(buff, start, end - start);
}

void	HTTPParseState::clearHeader()
{
	m_HeaderField.clear();
	m_HeaderValue.clear();
}

void	HTTPParseState::setState(HTTPParseState::state_t state)
{
	m_State = state;
}

void	HTTPParseState::setReadBytes(unsigned int val)
{
	m_ReadBytes = val;
}

char			HTTPParseState::getPrevChar() const
{
	return m_PrevChar;
}

HTTPParseState::chunkState	HTTPParseState::getChunkState() const
{
	return m_ChunkState;
}

unsigned int	HTTPParseState::getChunkPos() const
{
	return m_chunkPos;
}

void	HTTPParseState::incrementChunkPos(unsigned int n)
{
	m_chunkPos += n;
}

void	HTTPParseState::resetChunk()
{
	m_chunkPos = 0;
	m_ChunkSize = 0;
}

void			HTTPParseState::setPrevChar(const char c)
{
	m_PrevChar = c;
}

char			*HTTPParseState::getMethod()
{
	return m_Method;
}

void	HTTPParseState::setChunkState(HTTPParseState::chunkState newState)
{
	m_ChunkState = newState;
}

bool	HTTPParseState::isChunkComplete() const
{
	return m_ChunkSize == m_chunkPos;
}

size_t	HTTPParseState::getChunkSize() const
{
	return m_ChunkSize;
}

void	HTTPParseState::appendChunkSize(char c)
{
	m_ChunkSizeStr += c;
}

void	HTTPParseState::setError()
{
	m_State = PARSE_ERROR;
}

bool	HTTPParseState::validateChunkSize()
{
	std::cout << "chunk size string: " << m_ChunkSizeStr << std::endl;
	std::stringstream ss;

	ss << m_ChunkSizeStr;
	ss << std::hex;
	m_ChunkSizeStr.clear();
	return (ss >> m_ChunkSize) && ss.eof();
}

unsigned int	HTTPParseState::getReadBytes() const
{
	return m_ReadBytes;
}

HTTPParseState::state_t	HTTPParseState::advance(bool resetReadBytes)
{
	if (resetReadBytes)
		m_ReadBytes = 0;
	if (m_State != PARSE_DONE && m_State != PARSE_ERROR)
		m_State = static_cast<state_t>(m_State + 1);
	return m_State;
}

HTTPParseState::~HTTPParseState(void)
{
	
}