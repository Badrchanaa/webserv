#include <iostream>
#include <string>
#include <algorithm>

#include <fstream>
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

int main(int ac, char **av)
{
	std::string key("oOEWOFWEO oewofweo fewofowe--__--");
	if (ac < 2)
		return 0;
	std::string::iterator it = key.begin();
	it++;
	it++;
	it++;
	it++;
	std::string::iterator it2 = std::transform(key.begin(), key.end(), it, ::tolower);
	if (it2 == key.end())
	{
		std::cout << "is end" << std::endl;
	}
	else
	{
		std::cout << "it2: " << *it2 << std::endl;
	}
	std::cout << "key: " << key << std::endl;
	return 0;
}
