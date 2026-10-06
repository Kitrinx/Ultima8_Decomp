#ifndef SYSTEM_H
#define SYSTEM_H

class SimpleVirtualString
{
public:
	char *string;
	int size;

	SimpleVirtualString(void);
	SimpleVirtualString(short);
	~SimpleVirtualString(void);
	void free(void);
	char *alloc(short);
	void concatenate(const char *);
	void empty(void);
	char *operator=(const char *);
	char *operator+=(const char *);
	void swap(SimpleVirtualString &);
	char *assemble(const char *, ...);
	int operator--(void);
	operator char *(void);
};

extern SimpleVirtualString WorkString;

// Prints a caller's arguments into WorkString, or empties it.
#define FORMAT_WORKSTRING(format, args) \
	if (format && *format && format != (char *)WorkString) \
	{ \
		va_start(args, format); \
		vsprintf(WorkString, format, args); \
	} \
	else \
		*(char *)WorkString = 0

#endif
