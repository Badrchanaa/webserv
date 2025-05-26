#include <iostream>
#include <string>
#include <fstream>

int main()
{
	int a = (0x1) | (0x1 << 1) | (0x1 << 2);

	int b = (0x1) | (0x1 << 3);

	std::cout << "a : " << a << std::endl;
	std::cout << "b : " << b << std::endl;
	if ((a & b) == b)
	{
		std::cout << "valid" << std::endl;
	}
	else
		std::cout << "NOT valid" << std::endl;
	std::cout << static_cast<int>(a & b) << std::endl;
	return 0;
}
