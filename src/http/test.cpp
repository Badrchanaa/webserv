#include <unistd.h>
#include <iostream>
#include "HTTPRequest.hpp"
#include "HTTPParser.hpp"

# define BUFF_SIZE 32
int main()
{
    
    char    buff[BUFF_SIZE];
    int     rbytes;

    rbytes = read(0, buff, BUFF_SIZE);
    HTTPRequest request;
    HTTPParseState &parseState = request.getParseState();

    while (rbytes > 0)
    {
        std::cout << "got some bytes." << std::endl;

        HTTPParser::parse(request, buff, rbytes);
        if (parseState.isError())
        {
            std::cout << "PARSE ERROR" << std::endl;
        }
        else if (parseState.isComplete())
        {
            std::cout << "PARSE COMPLETE" << std::endl;
        }
        else
        {
            std::cout << "PARSE ONGOING" << std::endl;
        }
        std::cout << "Path: " << request.getPath() << std::endl;
        rbytes = read(0, buff, BUFF_SIZE);
    }
    
    return 0;
}