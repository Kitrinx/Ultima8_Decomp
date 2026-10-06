#ifndef HANDLER_H
#define HANDLER_H

#include "CACHE.H"

#define MAX_HANDLES	4000

// Owns a type of cache node: reserves a run of handles and loads
// the data behind them on demand.
class CacheHandler
{
public:
	CacheNodeType type;

	CacheHandler(CacheNodeType t) { type = t; }
	~CacheHandler(void);
	virtual void notifyMoved(unsigned long) {}
	virtual void cacheOut(unsigned long, unsigned) {}
	virtual unsigned char verify(unsigned long) { return 1; }
	static void init(void);
	static int reserve(int);
	static unsigned long *handles;
	static int numHandles;
};

void initCacheHandler(void);

#endif
