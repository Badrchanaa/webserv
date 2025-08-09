# webserv
A simple and efficient HTTP/1.1 server with built-in support for CGI scripts and file uploads.

# Features
- Static file servings
- HTTP/1.1: Handles GET, POST, DELETE, HEAD, PUT methods.
- Configurable: easy to configure with built-in support for yaml configuration files.
- Fast: Built in C++. Uses stream parsing and a single-thread event loop.
- File upload: Built-in support for file upload via multipart/form-data
- CGI: Built-in support for CGI scripts.

# Prerequisites
A C++98 Compiler & make

# Installation
git clone <repo_url> webserv
cd webserv
make

# Usage
./webserv [config_file.yaml]
If no configuration file is provided, the server will use the default settings.


