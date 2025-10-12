# webserv

A lightweight, high-performance HTTP/1.1 web server written in C++98. Built with a focus on efficiency and standards compliance, webserv provides a robust foundation for serving static content, handling file uploads, and executing CGI scripts.

## Features

- **HTTP/1.1 Compliance**: Full support for GET, POST, PUT, DELETE, and HEAD methods
- **Static File Serving**: Efficiently serve static HTML, CSS, JavaScript, images, and other assets
- **CGI Support**: Execute dynamic scripts written in Python, PHP, Perl, or compiled CGI binaries
- **File Upload**: Built-in multipart/form-data parsing for handling file uploads
- **YAML Configuration**: Flexible and readable configuration using YAML files
- **Event-Driven Architecture**: Single-threaded event loop using epoll for high performance
- **Virtual Hosts**: Support for multiple server blocks with different hostnames
- **Auto-indexing**: Optional directory listing for browsing file structures
- **Custom Error Pages**: Configure custom HTML pages for different HTTP error codes
- **Request Body Size Limits**: Configurable limits to prevent oversized uploads

## Architecture

webserv is designed with a modular architecture:

- **HTTP Parser**: Stream-based parser for handling HTTP requests incrementally
- **Configuration System**: YAML-based configuration with validation
- **Epoll Event Loop**: Non-blocking I/O using Linux epoll for handling multiple connections
- **CGI Handler**: Fork-based execution of CGI scripts with proper environment setup
- **Resource Manager**: File serving, upload handling, and directory operations

## System Requirements

### Build Requirements
- **C++ Compiler**: Any C++98 compliant compiler (g++, clang++)
- **Make**: GNU Make or compatible build system
- **Operating System**: Linux (requires epoll support)

### Runtime Dependencies
- **Core**: No external libraries required - uses only standard C++ and POSIX APIs
- **Optional CGI Interpreters** (if using CGI scripts):
  - Python 3.x (`/usr/bin/python3`)
  - PHP CGI (`/usr/bin/php-cgi`)
  - Perl (`/usr/bin/perl`)

### System Libraries Used
- Standard C++ Library (iostream, string, vector, map, etc.)
- POSIX APIs (socket, epoll, fork, exec)
- Standard C Library (cstring, cstdlib, cstdio)

## Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/Badrchanaa/webserv.git
   cd webserv
   ```

2. **Build the server**:
   ```bash
   make
   ```

   The build process will compile all source files and create the `webserv` executable.

3. **Verify the build**:
   ```bash
   ls -lh webserv
   ```

### Optional: Install CGI Interpreters

If you plan to use CGI scripts, install the required interpreters:

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install python3 php-cgi perl

# Fedora/RHEL
sudo dnf install python3 php-cgi perl

# Arch Linux
sudo pacman -S python php perl
```

## Configuration

webserv uses YAML configuration files to define server behavior. Configuration files support:

- **Server blocks**: Define multiple virtual hosts
- **Locations**: Configure URL path handling
- **CGI settings**: Map file extensions to interpreters
- **Upload directories**: Specify where uploaded files are stored
- **Error pages**: Customize error responses

### Configuration Example

```yaml
server:
  host: 127.0.0.1
  port: 8080
  timeout: 60
  server_name: localhost
  body_size: 10M
  
  errors:
    404: errors/404.html
    500: errors/500.html
    403: errors/403.html
  
  location:
    uri: /
    root: www
    index: index.html
    autoindex: on
    methods:
      - GET
      - POST
      - DELETE
    upload: uploads
  
  location:
    uri: /cgi-bin/
    root: www
    methods:
      - GET
      - POST
    methods_cgi:
      - GET
      - POST
    cgi:
      - .py: /usr/bin/python3
      - .php: /usr/bin/php-cgi
      - .pl: /usr/bin/perl
```

### Configuration Options

- `host`: IP address to bind to (default: 127.0.0.1)
- `port`: Port number to listen on (default: 8080)
- `timeout`: Request timeout in seconds
- `server_name`: Virtual host names (space-separated)
- `body_size`: Maximum request body size (e.g., "10M", "1G")
- `errors`: Map HTTP status codes to custom error pages
- `location.uri`: URL path prefix for this location block
- `location.root`: Document root directory
- `location.index`: Default index files
- `location.methods`: Allowed HTTP methods
- `location.autoindex`: Enable/disable directory listing ("on"/"off")
- `location.upload`: Directory for uploaded files
- `location.cgi`: Map file extensions to CGI interpreters

## Usage

### Basic Usage

Run the server with the default configuration:

```bash
./webserv
```

This will use the default configuration file at `./config/default.yml`.

### Custom Configuration

Specify a custom configuration file:

```bash
./webserv config/myconfig.yml
```

### Example Configuration Files

The repository includes several example configurations:

- `config/default.yml`: Default server configuration (port 4000)
- `config/test.yml`: Test configuration
- `config/d.yml`: Additional example configuration

### Testing the Server

1. **Start the server**:
   ```bash
   ./webserv config/default.yml
   ```

2. **Access via web browser**:
   ```
   http://localhost:4000/
   ```

3. **Test with curl**:
   ```bash
   # GET request
   curl http://localhost:4000/

   # POST request
   curl -X POST -d "data=value" http://localhost:4000/

   # File upload
   curl -X POST -F "file=@/path/to/file.txt" http://localhost:4000/upload
   ```

### CGI Scripts

Sample CGI scripts are provided in `www/app/cgi-bin/`:

- `hello.py`: Python CGI example
- `hello.php`: PHP CGI example
- `hello.pl`: Perl CGI example

Access them via:
```
http://localhost:4000/app/cgi-bin/hello.py
```

## Project Structure

```
webserv/
├── src/                  # Source code
│   ├── http/            # HTTP parsing and message handling
│   ├── Multiplixing/    # Epoll event loop and server core
│   ├── Cgi/             # CGI execution
│   └── Parsing/         # Configuration file parsing
├── includes/            # Header files
├── config/              # Configuration files
├── www/                 # Web content
│   ├── app/            # Sample application
│   ├── errors/         # Error pages
│   └── landing-page/   # Demo landing page
├── Makefile            # Build configuration
└── README.md           # This file
```

## Development

### Building with Debug Symbols

```bash
make DEBUG=1
```

This enables AddressSanitizer for memory debugging.

### Cleaning Build Files

```bash
make clean      # Remove object files
make fclean     # Remove object files and executable
make re         # Rebuild from scratch
```

## Troubleshooting

### Port Already in Use

If you see "Address already in use" error:

```bash
# Check which process is using the port
lsof -i :8080

# Kill the process or change the port in your config file
```

### Permission Denied

Ensure you have write permissions for:
- Upload directory (specified in config)
- Temporary file directory

### CGI Scripts Not Executing

1. Verify the interpreter path in your config matches the actual installation:
   ```bash
   which python3
   which php-cgi
   which perl
   ```

2. Ensure CGI scripts have execute permissions:
   ```bash
   chmod +x www/app/cgi-bin/*.py
   ```

3. Check that scripts have proper shebang line:
   ```python
   #!/usr/bin/env python3
   ```

### Configuration File Errors

If the server fails to start, check:
- YAML syntax is correct (proper indentation, no tabs)
- File paths exist and are readable
- Port number is available and > 1024 (for non-root users)

## Performance Considerations

- **Single-threaded**: One thread handles all connections using epoll
- **Non-blocking I/O**: All socket operations are non-blocking
- **Stream Parsing**: HTTP requests are parsed incrementally to minimize memory usage
- **Memory Management**: Careful resource cleanup to prevent leaks

## Standards Compliance

- **HTTP/1.1**: RFC 2616 / RFC 7230-7235
- **CGI/1.1**: RFC 3875
- **C++98**: ISO/IEC 14882:1998 standard

## License

This project is developed as part of the 42 School curriculum.

## Contributing

This is an educational project. Feel free to use it as a reference or learning material.


