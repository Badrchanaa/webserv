# webserv
A simple and efficient HTTP/1.1 server with built-in support for CGI scripts and file uploads.

# Features
- Static file servings
- HTTP/1.1: Handles GET, POST, DELETE methods.
- Configurable: easy to configure with built-in support for yaml configuration files.
- Efficient: Built in C++ with stream parsing and a single-threaded event loop for handling requests.
- File upload: Built-in support for file upload via multipart/form-data
- CGI Support: Full support for executing CGI (Common Gateway Interface) scripts

# Prerequisites
C++ Compiler

# Installation
git clone <repo_url> webserv
cd webserv
make

# Usage
./webserv [config_file.yaml]
If no configuration file is provided, the server will use the default settings.


