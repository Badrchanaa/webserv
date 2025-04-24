#include "HTTPParseState.hpp"
#include <iostream>
#include <string>

bool	HTTPParseState::isComplete() const
{
	return m_RequestState == REQ_DONE || m_RequestState == REQ_ERROR;
}

bool	HTTPParseState::isError() const
{
	return m_RequestState == REQ_ERROR;
}

HTTPParseState::requestState	HTTPParseState::getState() const
{
	return m_RequestState;
}

void	HTTPParseState::appendHeaderValue(const char *buff, size_t start, size_t end)
{
	if (start == end)
		return;
	m_HeaderValue.append(buff + start, end - start);
}

void	HTTPParseState::appendHeaderField(const char *buff, size_t start, size_t end)
{
	if (start == end)
		return;
	m_HeaderField.append(buff + start, end - start);
}

void	HTTPParseState::setState(HTTPParseState::requestState state)
{
	m_RequestState = state;
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

void			HTTPParseState::setPrevChar(const char c)
{
	m_PrevChar = c;
}

char			*HTTPParseState::getMethod()
{
	return m_Method;
}

std::string &HTTPParseState::getChunkSizeStr()
{
	return m_ChunkSizeStr;
}

unsigned int	HTTPParseState::getReadBytes() const
{
	return m_ReadBytes;
}

HTTPParseState::requestState	HTTPParseState::advance(bool resetReadBytes)
{
	if (resetReadBytes)
		m_ReadBytes = 0;
	if (m_RequestState != REQ_DONE)
		m_RequestState = static_cast<requestState>(m_RequestState + 1);
	return m_RequestState;
}

HTTPParseState::HTTPParseState(void): m_RequestState(REQ_LINE_START), m_ReadBytes(0)
{
}

HTTPParseState::HTTPParseState(const HTTPParseState &other)
{
	m_ReadBytes = other.m_ReadBytes;
	m_RequestState = other.m_RequestState;
}

HTTPParseState& HTTPParseState::operator=(const HTTPParseState &other)
{
	m_ReadBytes = other.m_ReadBytes;
	m_RequestState = other.m_RequestState;
	return *this;
}

HTTPParseState::~HTTPParseState(void)
{
	
}