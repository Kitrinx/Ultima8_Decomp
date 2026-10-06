#ifndef SCRIT_H
#define SCRIT_H

class SearchCriteria
{
public:
	unsigned char data[32];

	SearchCriteria(short, ...);
	int cmdlen(short);
	void operator+=(SearchCriteria);
	operator char *(void) { return (char *)data; }
};

#endif
