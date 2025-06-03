#include <iostream>
#include <string>
#include <algorithm>

#include <fstream>
#include <cstring>
#include <map>


void test(const char *str)
{
	std::string contentType(str);
	std::string::size_type pos = contentType.find(';');
	if (contentType.substr(0, pos) != "multipart/form-data")
		return ;
	std::map<std::string, std::string> mediaTypes; 
	std::string::size_type sepPos;
	// multi;ewfo=weof;weofe=owef
	while (pos != std::string::npos)
	{
		contentType = contentType.substr(pos + 1, std::string::npos);
		sepPos = contentType.find('=', 0);
		if (sepPos == std::string::npos)
			break ;
		pos = contentType.find(';', 0);
		std::cout << "pos: " << pos << "| sepPos: " << sepPos << std::endl;
		mediaTypes[contentType.substr(0, sepPos)] = contentType.substr(sepPos + 1, pos - sepPos - 1);
	}
	for (std::map<std::string, std::string>::const_iterator it = mediaTypes.begin(); it != mediaTypes.end(); it++)
	{
		std::cout << it->first << ": " << it->second<< std::endl;
	}
	// sepPos = contentType.find('=', 0);
	// if (sepPos == std::string::npos)
	// 	return;
	// name = contentType.substr(0, sepPos);
	// value = contentType.substr(sepPos + 1, pos);
	// 	std::cout << "[!]name: " << name << " | value: " << value << std::endl;
}

bool _validHeaderField(const int &c)
{
	if (c < 0 || c >= 128 || c == '\n')
		return true;
	if (c == ':')
		return true;
	return false;
}

// int main(int ac, char **av)
// {
// 	int		c;
// 	size_t	i;
// 	bool isError;
// 	const char buff[] = "HTTP/1.1";
// 	size_t start = 0;
// 	size_t len = std::strlen(buff);
// 	const char *http = "HTTP";
// 	size_t count = 0;
// 	const char *it = std::find_end(buff + start, buff + len, http + count, http + 4);
// 	i = std::distance(buff, it);
// 	if (i == len)
// 		std::cout << "not complete" << std::endl;
// 	else
// 	{
// 	if (*it != ':')
// 		isError = true;
// 	}
// 	std::cout << "i: " << i << std::endl;
// 	std::cout << "*it: " << *it << std::endl;
// 	std::cout << "isError: " << isError << std::endl;
// 	std::cout << "isPrintCr: " << std::isprint('\r') << std::endl;
// 	return 0;
// }
#include <vector>
int main() {
    const char* request_version = "HTTP/1.1";
    const char* pattern = "HTTP";
    const char* pat_begin;
	const char* pat_end;
	const char* req_begin;
	const char* req_end;
	const char* found;
	int chunkSize;
	int start;

	start = 0;
	chunkSize = 2;
    req_begin = request_version + start;
    req_end = request_version + chunkSize;
	pat_begin = pattern + start;
	if (chunkSize < std::strlen(pattern) - start)
    	pat_end = pattern + chunkSize;
	else
    	pat_end = pattern + std::strlen(pattern) - start;

    found = std::find_end(req_begin, req_end, pat_begin, pat_end);

    if (found != req_end) {
        std::cout << "\"HTTP\" found at position: " << (found - req_begin) << std::endl;
    } else {
        std::cout << "\"HTTP\" not found.\n";
    }

	start = chunkSize;
	chunkSize = std::strlen(request_version);
    req_begin = request_version + start;
    req_end = request_version + chunkSize;
	pat_begin = pattern + start;
	if (chunkSize < std::strlen(pattern) - start)
    	pat_end = pattern + chunkSize;
	else
    	pat_end = pattern + std::strlen(pattern) - start;

    found = std::find_end(req_begin, req_end, pat_begin, pat_end);

    if (found != req_end) {
        std::cout << "\"HTTP\" found at position: " << (found - req_begin) << std::endl;
    } else {
        std::cout << "\"HTTP\" not found.\n";
    }
    return 0;
}
