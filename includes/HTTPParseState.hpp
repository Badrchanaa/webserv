#ifndef __HTTPParseState_HPP__
# define __HTTPParseState_HPP__

#include <string>

/// @brief HTTP Request/CGI parsing state class.
class HTTPParseState
{
	public:
		typedef	enum
		{
			//  request-line   = method SP request-target SP HTTP-version
			PARSE_LINE_START = 1,
			PARSE_LINE_METHOD = 2,
			PARSE_LINE_TARGET = 3,
			PARSE_LINE_HTTP = 4,
			PARSE_LINE_VERSION_MINOR = 5,
			PARSE_LINE_VERSION_DOT = 6,
			PARSE_LINE_VERSION_MAJOR = 7,

			PARSE_HEADER_CRLF = 8,
			PARSE_HEADER_FIELD = 9,
			PARSE_HEADER_VALUE = 10,
			PARSE_BODY_CRLF = 11,
			PARSE_BODY = 12,
			// PARSE_MULTIPART_BODY_CRLF = 13,
			PARSE_MULTIPART_BOUNDARY = 14,
			PARSE_MULTIPART_BOUNDARY_SUFFIX = 15,
			PARSE_DONE = 16,
			PARSE_ERROR = 17,
		}		state_t;

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
		state_t			getState() const;
		chunkState		getChunkState() const;
		state_t			getMultipartState() const
		{
			return m_MultipartState;
		}
		void			setState(HTTPParseState::state_t state);
		void			setChunkState(HTTPParseState::chunkState newState);
		unsigned int	getReadBytes() const;
		void			setReadBytes(unsigned int val);
		char			getPrevChar() const;
		void			setPrevChar(const char c);
		void			setError();
		/// @brief advances parsing state to next state and resets read bytes counter to zero.
		/// @param resetReadBytes
		/// @return new state
		state_t			advance(bool resetReadBytes=true);
		bool			isComplete() const;
		bool			isError() const;
		void			appendHeaderField(const char *buff, size_t start, size_t end);
		void			appendHeaderValue(const char *buff, size_t start, size_t end);
		void			clearHeader();

		std::string		getHeaderField() const;
		std::string		getHeaderValue() const;

		void			appendChunkSize(const char c);
		bool			validateChunkSize();
		bool			isChunkComplete() const;
		char*			getMethod();
		size_t			getChunkSize() const;
		unsigned int	getChunkPos() const;
		void			incrementChunkPos(unsigned int n);
		void			resetChunk();
		~HTTPParseState();
	private:
		std::string			m_HeaderField;
		std::string			m_HeaderValue;

		char				m_PrevChar;
		state_t				m_State;
		char				m_Method[10];
		unsigned int		m_ReadBytes;
		std::string			m_ChunkSizeStr;
		unsigned int		m_chunkPos;
		size_t				m_ChunkSize;
		chunkState			m_ChunkState;
		state_t				m_MultipartState;

};

#endif