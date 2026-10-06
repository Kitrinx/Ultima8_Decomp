#ifndef OCCLUDE_H
#define OCCLUDE_H

// Highest roof z over each 32-unit cell of the 9x9 loaded regions.
#define ZBUFFER_SIZE	144

class Zbuffer
{
public:
	unsigned char (*buffer)[ZBUFFER_SIZE];

	Zbuffer(void);
	~Zbuffer(void);
	void alloc(void);
	void init(void);
	void make(void);
	unsigned char isHidden(unsigned short);
};

#endif
