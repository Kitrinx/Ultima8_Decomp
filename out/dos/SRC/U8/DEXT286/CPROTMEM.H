#ifndef CPROTMEM_H
#define CPROTMEM_H

#include <alloc.h>

// A descriptor whose bytes 6 and 7 hold a list index.
class FreeDescriptor
{
public:
	unsigned char pad_00[6];
	unsigned char indexLow;
	unsigned char indexHigh;

	unsigned short getIndex(void);
	void setIndex(unsigned short);
};

#define BLOCK_HEADER 10UL

// Header of a block of linear memory, reached through makePtr().
// Blocks are laid end to end; a free block's size is its length - 1 (odd),
// a used block's is its length, with its owner tag after prevBlock.
class FreeBlock
{
public:
	unsigned long size;
	unsigned long prevBlock;	// block before this one in memory
	union {
		unsigned long prev;	// free list links, free blocks only
		unsigned short tag;	// owner, used blocks only
	};
	unsigned long next;

	unsigned long getNext(void);
	unsigned long getLength(void) { return size + 1; }
	unsigned char isFree(void) { return size & 1; }
	unsigned char isEnd(void) { return size == 0xFFFFFFFFL; }
	unsigned long getSize(void) { return size & 0x0FFFFFFFL; }
	void setSize(unsigned long s) { size = (size & 0xF0000000L) | s; }
	void setFreeLength(unsigned long len) { size = len - 1; }
	void setTag(unsigned short t) { tag = t; }
	void setPrevBlock(unsigned long p) { prevBlock = p; }
	void setPrev(unsigned long p) { prev = p; }
	void setNext(unsigned long n) { next = n; }
};

// One range of linear memory with its own free list.
class Fragment
{
public:
	unsigned long start;	// list head block
	unsigned long length;
	unsigned short used;
	unsigned short freeBlocks;

	unsigned long allocate(unsigned long, unsigned short);
	void free(void *, unsigned short);
	void garbageCollect(void);
	void verify(char *, int);
	void getMemStatus(unsigned long &, unsigned long &);
	unsigned char contains(unsigned long a) { return start <= a && start + length >= a; }
};

#define MAX_FRAGMENTS 10

class ProtMemoryManager
{
public:
	static void verify(char *, int);
	static void *allocate(unsigned long, unsigned short, unsigned char);
	static void garbageCollect(unsigned char);
	static void free(void *, unsigned short);
	static void allocateFragments(void);
	static void freeFragments(void);
	static void getMemStatus(short, unsigned long &, unsigned long &);
	static void getMemStatus(unsigned long &, unsigned long &, unsigned short &);
	static void initialize(void);
	static unsigned char initialized;
	static unsigned char fromLargest;
	static unsigned char failedLastRequest;
	static Fragment fragments[MAX_FRAGMENTS];
	static unsigned short numFragments;
};

void pascal CLEANUP(unsigned short);

extern unsigned FRAGMENT_0_PERCENTAGE;
extern unsigned char newAddress1[4];
extern unsigned char newAddress2[4];
extern unsigned char reserved[1];
extern unsigned char addr_xhi[1];

#endif
