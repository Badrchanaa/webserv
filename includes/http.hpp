#ifndef __HTTP_HPP__
# define __HTTP_HPP__

typedef enum {
  METHOD_NONE = 0,
  GET = 1 << 0,
  POST = 1 << 1,
  DELETE = 1 << 2,
  PUT = 1 << 3
} httpMethod;

#endif