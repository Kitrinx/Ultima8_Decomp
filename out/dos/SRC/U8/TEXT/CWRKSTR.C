// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: TEXT\CWRKSTR.C

#include <alloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "SYSTEM.H"

// Inline in the shared header when this file was built.
inline SimpleVirtualString::SimpleVirtualString(short n) { string = 0; alloc(n); }
inline SimpleVirtualString::~SimpleVirtualString(void) { free(); }
inline SimpleVirtualString::operator char *(void) { return string; }

SimpleVirtualString WorkString(1024);

void SimpleVirtualString::free(void)
{
	if (string)
		delete string;
	string = 0;
	size = 0;
}

// Takes what memory is left if n bytes are not.
char *SimpleVirtualString::alloc(short n)
{
	free();
	if (coreleft() < n)
		n = coreleft();
	if (n > 0)
	{
		string = new char[n];
		*string = 0;
	}
	else
		string = 0;
	size = n;
	return string;
}

// Appends as much of s as fits.
void SimpleVirtualString::concatenate(const char *s)
{
	char *end;
	int left;

	if (string)
	{
		end = string;
		left = size;
		while (*end)
		{
			end++;
			left--;
		}
		while (left && *s)
		{
			*end = *s;
			s++;
			end++;
			left--;
		}
		if (left)
			*end = 0;
		else
			string[size - 1] = 0;
	}
}

void SimpleVirtualString::empty(void)
{
	if (string)
		*string = 0;
}

char *SimpleVirtualString::operator=(const char *s)
{
	int length;

	if (!s)
	{
		free();
		alloc(0);
	}
	else if (string == s)
		return string;
	length = _fstrlen(s) + 1;
	if (!string || size < length)
	{
		free();
		alloc(length);
	}
	if (string)
	{
		empty();
		concatenate(s);
		string[size - 1] = 0;
	}
	return string;
}

// Grows the buffer when memory allows, else keeps what fits.
char *SimpleVirtualString::operator+=(const char *s)
{
	int length;
	int newSize;

	if (s)
	{
		length = _fstrlen(s);
		length = strlen(string) + length + 1;
		newSize = size > length ? size : length;
		if (size < newSize && coreleft() > size)
		{
			SimpleVirtualString bigger(newSize);
			if (string)
				bigger.concatenate(string);
			bigger.concatenate(s);
			swap(bigger);
		}
		else
			concatenate(s);
	}
	return string;
}

void SimpleVirtualString::swap(SimpleVirtualString &other)
{
	char *otherString;
	int otherSize;

	otherString = string;
	otherSize = size;
	string = other.string;
	size = other.size;
	other.string = otherString;
	other.size = otherSize;
}

char *SimpleVirtualString::assemble(const char *format, ...)
{
	va_list args;

	FORMAT_WORKSTRING(format, args);
	empty();
	*this = WorkString;
	return string;
}

// Drops the last character.
int SimpleVirtualString::operator--(void)
{
	string[--size - 1] = 0;
	return (int)string;
}
