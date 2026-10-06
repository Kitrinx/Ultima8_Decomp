#ifndef CACHE_H
#define CACHE_H

#include "CDESC.H"

// Each cache node starts with a 29-byte header; handles point past it.
#define NODE_HEADER	29
// Handle of a free node, and of a node no handle points at.
#define FREE_HANDLE	0xfffe
#define NO_HANDLE	0xfffd
#define FREE_TYPE	((CacheNodeType)0xff)

class CacheHandler;

enum CacheNodeType
{
	CacheNodeType_0
};

// The header in front of every cache node.
struct CacheNode
{
	CacheNodeType type;
	unsigned handle;
	long size;
	unsigned char sizeDiff;
	long sum;
	long next;
	long prev;
	long less;
	long more;
};

class CacheNodePtr
{
public:
	unsigned long offset;

	CacheNodePtr(void) {}
	CacheNodePtr(unsigned long o) { offset = o; }
	static void *getPtr(unsigned long o) { return makePtr(base + o); }
	void free(void) { deallocate(*this); }
	unsigned char isFree(void) { return getHandle() == FREE_HANDLE; }
	unsigned char isAllocated(void) { return getHandle() < FREE_HANDLE; }
	unsigned char isDummy(void) { return getHandle() == 0xffff; }

	void freeSpaceToBlockMru(void);
	void makeMru(void);
	int unlinkFromMruThread(void);
	void unlinkFromBlockThread(void);
	void blockThreadInsertAfter(CacheNodePtr);
	void mruThreadReplace(CacheNodePtr);
	void addToFreeChain(void);
	void setMergeLink(CacheNodePtr);
	void reset(CacheNodePtr);
	void shiftUpSet(CacheNodePtr);
	static void setDummyNodes(void);
	long getSum(void);
	long getSize(void);
	long getLess(void);
	long getMore(void);
	long getNext(void);
	long getPrev(void);
	int getHandle(void);
	unsigned getSizeDiff(void);
	int getType(void);
	void setSize(long);
	void setSum(long);
	void setNext(CacheNodePtr);
	void setPrev(CacheNodePtr);
	void setType(CacheNodeType);
	void setSizeDiff(unsigned char);
	void setHandle(unsigned);
	void *shiftUpAsm(long);
	void shiftFreeBlock(long);
	static void initCache(long);
	static void uninitCache(void);
	static void allocateCache(void);
	static void attach(CacheHandler *, CacheNodeType);
	static void detach(CacheNodeType);
	static void createDummyNodes(void);
	static CacheNodePtr findSpace(long);
	static long allocate(long, unsigned, CacheNodeType);
	static void findLargestFreeNode(void);
	static long deallocate(CacheNodePtr);
	static void externalDeallocate(unsigned long);
	static void release(long);
	static void garbageCollect(long);
	void merge(void);
	void shiftUp(long);
	void shiftUpToFreeBlock(void);
	static int deallocateLru(CacheNodeType);
	static void verifyFailed(unsigned short, char *, unsigned short);
	static void verify(char *, unsigned short);
	static void dumpCache(unsigned char);
	void dump(void);
	static void profileCache(void);
	static int getPercentFull(void);
	static void save(char *);
	static void load(char *);
	static void *memBase;
	static CacheNodePtr cache;
	static CacheNodePtr blockRoot;
	static CacheNodePtr freeRoot;
	static CacheNodePtr largestFreeNode;
	static long totalBytes;
	static long freeBytes;
	static unsigned totalBlocks;
	static unsigned totalAllocatedBlocks;
	static unsigned totalFreeBlocks;
	static long largestFreeBytes;
	static long allocatedBytes;
	static long slop;
	static unsigned garbageCount;
	static unsigned long base;
	static unsigned numHandlers;
	static unsigned char hasUninit;
	static unsigned initialized;
	static CacheHandler *handlers[7];
};

void initCache(long);
void pascal UNINITCACHE(unsigned short);
void uninitCache(void);
void verifyCache(char *, short);
void profileCache(void);
void dump(void);

extern unsigned haltOnFail;

#endif
