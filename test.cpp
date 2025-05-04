
HTTPParseState:
	virtual methodPtr  onHeaderComplete;
	virtual onBodyData;
	virtual onBodyComplete;
	virtual isComplete(); ??
	virtual isError(); ??

HTTPParser:
	parse(HTTPParseState, BUFF, LEN);

RequestParser:
	parse(RequestParseState, buff, len);

CGIParser:
	parse(CGIParseState, buff, len);

HTTPMessage:
	headers
	body

	onHeaderData();
	onHeaderComplete();
	onBodyComplete();
Request:
	RequestParseState


Response:
	CGIParseState

RequestParseState: public HTTPParseState
	chunkState;
	setMethod (){}
	setPath (){}

CGIParseState: public HTTPParseState

