#ifndef CMEMHNDP_H
#define CMEMHNDP_H

class MemHandle
{
public:
	unsigned long f_00;
	unsigned char f_04;
	unsigned long f_05;

	MemHandle(void);
	MemHandle(char *);
	MemHandle(unsigned long, unsigned char);
	~MemHandle(void);
	void alloc(unsigned long, unsigned char);
	void alloc(unsigned long, unsigned char, short);
	void alloc(unsigned long, unsigned char, char *);
	void free(short);
	void copy(MemHandle &);
	char *get_ptr(void);
	void init(char *, unsigned char);
	void init(MemHandle &);
	void take_ownership(MemHandle &);
	int is_valid(void);
};

#endif
