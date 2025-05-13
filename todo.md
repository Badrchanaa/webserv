## ConfigServer class:
	- const Location&  getLocation(const std::string path);

## Location class:
	- bool  isMethodAllowed(httpMethod method);
	- const std::string	getIndexPath(const std::string path);
	- errors: change it to map<int, string>. Key should be an integer or remove quotes from key.
	- add index to location in config
