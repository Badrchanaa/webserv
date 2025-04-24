#ifndef __HTTPParseState_HPP__
# define __HTTPParseState_HPP__

#include <string>

/// @brief HTTP Request parsing state class.
class HTTPParseState
{
	public:
		typedef	enum e_RequestState
		{
			//  request-line   = method SP request-target SP HTTP-version
			REQ_LINE_START = 1,
			REQ_LINE_METHOD,
			REQ_LINE_TARGET,
			// REQ_LINE_H,
			// REQ_LINE_HT,
			// REQ_LINE_HTT,
			REQ_LINE_HTTP,
			REQ_LINE_VERSION_MINOR,
			REQ_LINE_VERSION_DOT,
			REQ_LINE_VERSION_MAJOR,
			REQ_HEADER_CRLF,
			REQ_HEADER_FIELD,
			REQ_HEADER_VALUE,
			REQ_BODY_CRLF,
			REQ_BODY,
			REQ_DONE,
			REQ_ERROR,
		}		requestState;
	public:
		HTTPParseState(void);
		HTTPParseState(const HTTPParseState &other);
		HTTPParseState& operator=(const HTTPParseState &other);
		requestState	getState() const;
		void			setState(HTTPParseState::requestState state);
		unsigned int	getReadBytes() const;
		void			setReadBytes(unsigned int val);
		char			getPrevChar() const;
		void			setPrevChar(const char c);
		/// @brief advances parsing state to next state and resets read bytes counter to zero.
		/// @param resetReadBytes
		/// @return new state
		requestState	advance(bool resetReadBytes=true);
		bool			isComplete() const;
		bool			isError() const;
		void			appendHeaderField(const char *buff, size_t start, size_t end);
		void			appendHeaderValue(const char *buff, size_t start, size_t end);
		std::string		&getHeaderField() const;
		std::string		&getHeaderValue() const;

		char			*getMethod();
		~HTTPParseState();
	private:
		std::string		m_HeaderField;
		std::string		m_HeaderValue;
		
		requestState	m_RequestState;
		char			m_Method[10];
		unsigned int	m_ReadBytes;
		char			m_PrevChar;

		
};

#endif