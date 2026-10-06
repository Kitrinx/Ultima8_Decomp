#ifndef YAMM_H
#define YAMM_H

class BaseFile;

#define YAMM_SIZE 20000

// Heap block header; a handle is the offset just past it.
struct YammBlock
{
	unsigned next;
	unsigned prev;
	unsigned pid;
	unsigned size;	// top bit set when free

	unsigned char isFree(void);
	unsigned getSize(void);
	void setFree(void);
	void setUsed(void);
	void grow(unsigned);
	void setSize(unsigned);
};

// List node: next handle, then the element (a string handle in string lists).
struct YammNode
{
	unsigned next;
	unsigned data;
};

class Yamm
{
public:
	static void *resolve(unsigned short);
	static void setPid(unsigned short, unsigned short);
	static int flimFlam(char *);
	static int flimFlam(unsigned short);
	static void clean(unsigned short);
	static void scram(unsigned short);
	static void heyMan(void);
	static void damn(unsigned short, char *, unsigned short);
	static void shazamm(char *, unsigned short);
	static void load(BaseFile &);
	static void save(BaseFile &);
	static unsigned numBlocks;
	static char *buffer;
	static YammBlock *head;
	static unsigned currentUnkPid;
};

class YammList
{
public:
	unsigned list;
	unsigned size;
	unsigned char isString;

	YammList(void) { list = 0; }
	YammList(unsigned short, unsigned short, int);
	void set(unsigned short l, unsigned short s, int str) { list = l; size = s; isString = str; }
	void kill(void);
	unsigned char traverse(unsigned short &);
	unsigned operator=(YammList &);
	void *operator[](int);
	char *resolveString(unsigned short);
	void operator+=(unsigned short);
	void operator^=(unsigned short);
	void operator-=(unsigned short);
	int in(char *);
	void setElement(int, char *);
	void changePid(unsigned short);
};

int initYamm(void);
void uninitYamm(void);
void printYammLongList(unsigned short);
void printYammIntList(unsigned short);
void printYammStringList(unsigned short);
void f7key(void);
void f10key(void);

#endif
