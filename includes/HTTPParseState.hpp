#ifndef __HTTPParseState_HPP__
# define __HTTPParseState_HPP__

#include <string>

/// @brief HTTP Request parsing state class.
class HTTPParseState
{
	public:
		typedef	enum requestState
		{
			//  request-line   = method SP request-target SP HTTP-version
			REQ_LINE_START = 1,
			REQ_LINE_METHOD = 2,
			REQ_LINE_TARGET = 3,
			REQ_LINE_HTTP = 4,
			REQ_LINE_VERSION_MINOR = 5,
			REQ_LINE_VERSION_DOT = 6,
			REQ_LINE_VERSION_MAJOR = 7,
			REQ_HEADER_CRLF = 8,
			REQ_HEADER_FIELD = 9,
			REQ_HEADER_VALUE = 10,
			REQ_BODY_CRLF = 11,
			REQ_BODY = 12,
			REQ_DONE = 13,
			REQ_ERROR = 14,
		}		requestState;
		typedef enum chunkState
		{
			CHUNK_SIZE = 0,
			CHUNK_EXT,
			CHUNK_CRLF,
			CHUNK_DATA,
			CHUNK_DATA_CRLF,
			CHUNK_END
		} chunkState;
	public:
		HTTPParseState(void);
		HTTPParseState(const HTTPParseState &other);
		HTTPParseState& operator=(const HTTPParseState &other);
		requestState	getState() const;
		chunkState		getChunkState() const;
		void			setState(HTTPParseState::requestState state);
		void			setChunkState(HTTPParseState::chunkState newState);
		unsigned int	getReadBytes() const;
		void			setReadBytes(unsigned int val);
		char			getPrevChar() const;
		void			setPrevChar(const char c);
		void			setError();
		/// @brief advances parsing state to next state and resets read bytes counter to zero.
		/// @param resetReadBytes
		/// @return new state
		requestState	advance(bool resetReadBytes=true);
		bool			isComplete() const;
		bool			isError() const;
		void			appendHeaderField(const char *buff, size_t start, size_t end);
		void			appendHeaderValue(const char *buff, size_t start, size_t end);
		std::string		getHeaderField() const;
		std::string		getHeaderValue() const;

		void			appendChunkSize(const char c);
		bool			validateChunkSize();
		bool			isChunkComplete() const;
		char*			getMethod();
		size_t			getChunkSize() const;
		unsigned int	getchunkPos() const;
		void			setchunkPos(unsigned int n);
		~HTTPParseState();
	private:
		std::string			m_HeaderField;
		std::string			m_HeaderValue;

		requestState		m_RequestState;
		char				m_Method[10];
		unsigned int		m_ReadBytes;
		char				m_PrevChar;
		chunkState			m_ChunkState;
		std::string			m_ChunkSizeStr;
		unsigned int		m_chunkPos;
		size_t				m_ChunkSize;

};

#endif