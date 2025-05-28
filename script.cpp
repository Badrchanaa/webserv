#include <iostream>
#include <cstdlib>
#include <string>

int main() {
    std::string method = getenv("REQUEST_METHOD") ? getenv("REQUEST_METHOD") : "";
    std::string query = getenv("QUERY_STRING") ? getenv("QUERY_STRING") : "";

    std::string post_data;
    if (method == "POST") {
        int content_length = std::atoi(getenv("CONTENT_LENGTH"));
        char *buffer = new char[content_length + 1];
        std::cin.read(buffer, content_length);
        buffer[content_length] = '\0';
        post_data = buffer;
        delete[] buffer;
    }

    std::cout << "Content-Type: text/html\r\n\r\n";
    std::cout << "<html><body>";
    std::cout << "<h1>C++ CGI Test</h1>";
    std::cout << "<p><b>Method:</b> " << method << "</p>";
    std::cout << "<p><b>Query:</b> " << query << "</p>";
    std::cout << "<p><b>POST Data:</b> " << post_data << "</p>";
    std::cout << "</body></html>";

    return 0;
}

