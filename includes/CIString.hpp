#ifndef __CISTRING_HPP__
# define __CISTRING_HPP__

#include <cstring>
#include <string>

class CaseInsensitiveTraits : public std::char_traits<char>
{
	static bool eq(char a, char b) { return toupper(a) == toupper(b); }
    static bool lt(char a, char b) { return toupper(a) < toupper(b); }
    static int compare(const char* s1, const char* s2, size_t n) {
        return strncasecmp(s1, s2, n);
	}
};

typedef std::basic_string<char, CaseInsensitiveTraits> CIString;

#endif