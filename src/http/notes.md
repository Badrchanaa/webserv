When headers complete:
- get config server
- do not wait for body if method is GET or not allowed
- if content-length is set and transfer-encoding:chunked return 400 bad request
