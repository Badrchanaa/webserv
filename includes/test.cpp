#include <iostream>
#include <fstream>
#include <sstream>

int main()
{
	std::iostream *stream;
	std::iostream *stream2;
	char buff[10];

	stream = new std::fstream("newfile.cpp", std::fstream::in | std::fstream::out | std::fstream::app);
	stream2 = new std::stringstream;

	stream2->write("this is a test", 10);
	stream2->read(buff, 10);


	stream->write(buff, 10);
	delete stream;
	delete stream2;
	return 0;
}
