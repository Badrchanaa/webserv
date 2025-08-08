## ConfigServer class:
	- const Location&  getLocation(const std::string path); --> Done For testing

## Location class:
	- bool  isMethodAllowed(httpMethod method); --> Done 
	- const std::string	getIndexPath(const std::string path); --> Done for testing only, it's need more parsing
	- errors: change it to map<int, string>. Key should be an integer or remove quotes from key.--> Done you can use "888" or 888  in the config file.
	- add index to location in config --> Done for testing only, it's need more parsing, check for Valid indexs and support Multi indexes.




i add this things:

getCgiStderrFd -> Response class
    void processStderr();
    std::string m_StderrBuffer;  // store stderr content
    bool m_HasStderrData;