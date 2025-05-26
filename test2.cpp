#include <iostream>

std::string &getStatus()
{
	std::string	statusStr("404 Not Found");
	return statusStr;
}

int main()
{
	std::string statusStr = getStatus();
	const char *strc = statusStr.c_str();
	long n = std::strtol(strc, NULL, 10);

	std::cout << "n=" << n << std::endl;
	return 0;
}
