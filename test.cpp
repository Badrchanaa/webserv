#include <iostream>
#include <string>
#include <fstream>

int main()
{
	std::fstream fs;

	fs.open("Makefile");

	std::cout << "available bytes: " << fs.rdbuf()->in_avail() << std::endl;
	return 0;
}
