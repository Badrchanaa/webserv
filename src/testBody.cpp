#include <string.h>
#include <iostream>


int main() {
	const char *buff = "testbuff";
	
	const char *boundary = "tistbuff";
	const char *it = std::mismatch(buff, buff + strlen(buff), boundary).second;
	size_t pos = std::distance(boundary, it);
	std::cout << "mismatch at " << pos <<  std::endl;
    return 0;
}
