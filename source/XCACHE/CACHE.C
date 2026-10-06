// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\XCACHE\CACHE.C

#include <conio.h>
#include <dos.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>
#include <phapi.h>
#include "CEXIT.H"
#include "CPROTMEM.H"
#include "HANDLER.H"
#include "SHAPHAND.H"
#include "ERROR.H"
#include "ANIM.H"

#define DEFAULT_CACHE	0x800000L
#define CLEAR_CHUNK	64000L

// Three dummy nodes anchor the block, free and MRU threads.
#define DUMMY_BYTES	(3 * NODE_HEADER)

CacheNodePtr CacheNodePtr::cache(0);
CacheNodePtr CacheNodePtr::blockRoot(0);
CacheNodePtr CacheNodePtr::freeRoot(0);
CacheNodePtr CacheNodePtr::largestFreeNode(0);
long CacheNodePtr::totalBytes = 0;
long CacheNodePtr::freeBytes = 0;
unsigned CacheNodePtr::totalBlocks = 0;
unsigned CacheNodePtr::totalAllocatedBlocks = 0;
unsigned CacheNodePtr::totalFreeBlocks = 0;
long CacheNodePtr::largestFreeBytes = 0;
long CacheNodePtr::allocatedBytes = 0;
long CacheNodePtr::slop = 0;
unsigned CacheNodePtr::garbageCount = 0;
unsigned long CacheNodePtr::base = 0;
unsigned CacheNodePtr::numHandlers = 0;
unsigned char CacheNodePtr::hasUninit = 0;
unsigned CacheNodePtr::initialized = 0;
unsigned haltOnFail = 1;
CacheHandler *CacheNodePtr::handlers[7];

void initCache(long size)
{
	if (size > DEFAULT_CACHE || size == 0)
		size = DEFAULT_CACHE;
	CacheNodePtr::initCache(size);
}

void pascal UNINITCACHE(unsigned short)
{
	CacheNodePtr::uninitCache();
	DosExitList(EXLST_EXIT, 0);
}

void uninitCache(void)
{
	CacheNodePtr::uninitCache();
}

void verifyCache(char *file, short line)
{
	CacheNodePtr::verify(file, line);
}

void profileCache(void)
{
	CacheNodePtr::profileCache();
}

void dump(void)
{
	asm {
		mov al, 0x83
		mov ah, 0
		int 0x10
	}
	textmode(C80);
	textmode(C4350);
	CacheNodePtr::dumpCache(0);
}

void CacheNodePtr::initCache(long size)
{
	totalBytes = size;
	allocateCache();
	createDummyNodes();
	numHandlers = 0;
	slop = 0;
	initialized = 1;
	hasUninit = 0;
	DosExitList(EXLST_ADD, UNINITCACHE);
}

void CacheNodePtr::uninitCache(void)
{
	if (!hasUninit)
	{
		hasUninit = 1;
		CacheNodePtr node = blockRoot;
		for (unsigned i = 0; i < totalBlocks; i++)
		{
			CacheNodePtr next = node.getMore();
			if (node.getType() <= 6)
				deallocate(node);
			node = next;
		}
		if (memBase)
			farfree(memBase);
	}
}

void CacheNodePtr::allocateCache(void)
{
	char *block;
	unsigned long total, free;
	ProtMemoryManager::fragments[0].getMemStatus(total, free);
	totalBytes = free - 0x1000;
	memBase = ProtMemoryManager::allocate(totalBytes, 0, 0);
	if (memBase == 0)
		halt(__FILE__, 187);	// __LINE__
	Desc *desc = LocalDescriptorTable::getDesc(FP_SEG(memBase));
	base = desc->getBase();
	cache.offset = 0;
	int chunks = totalBytes / CLEAR_CHUNK;
	for (long i = 0; i < chunks; i++)
	{
		block = (char *)getPtr(i * CLEAR_CHUNK);
		memset(block, 0, CLEAR_CHUNK);
	}
	block = (char *)getPtr(i * CLEAR_CHUNK);
	memset(block, 0, totalBytes % CLEAR_CHUNK);
}

void CacheNodePtr::attach(CacheHandler *handler, CacheNodeType type)
{
	if (numHandlers < 7)
	{
		handlers[type] = handler;
		numHandlers++;
	}
	else
		halt(__FILE__, 213);	// __LINE__
}

void CacheNodePtr::detach(CacheNodeType type)
{
	handlers[type] = 0;
}

void CacheNodePtr::createDummyNodes(void)
{
	freeBytes = totalBytes - DUMMY_BYTES;
	largestFreeBytes = freeBytes;
	totalBlocks = 2;
	totalAllocatedBlocks = 1;
	totalFreeBlocks = 1;
	blockRoot = cache;
	freeRoot = cache.offset + NODE_HEADER;
	CacheNodePtr mruRoot;	// never used, but it has a frame slot
	setDummyNodes();
}

// Finds a free node of at least size bytes, making room first if needed.
CacheNodePtr CacheNodePtr::findSpace(long size)
{
	if (totalBytes - DUMMY_BYTES <= size)
		halt(__FILE__, 253);	// __LINE__
	CacheNodePtr largest = largestFreeNode;
	if (size > largestFreeBytes || largest.getHandle() != FREE_HANDLE)
	{
		if (freeBytes + slop >= size)
		{
			if (totalFreeBlocks > 0)
				garbageCollect(size);
			else
				release(size + NODE_HEADER);
		}
		else
			release(size);
	}
	CacheNodePtr node = freeRoot;
	node = node.getLess();
	do
	{
		if (node.getSize() >= size)
			return node;
		node = node.getLess();
	} while (node.offset != freeRoot.offset);
	halt(__FILE__, 291);	// __LINE__
	return CacheNodePtr(0);
}

// Takes a node for size bytes and returns the offset of its data.
long CacheNodePtr::allocate(long size, unsigned handle, CacheNodeType type)
{
	CacheNodePtr node = findSpace(size);
	long nodeSize = node.getSize();
	long data;
	if (size + NODE_HEADER < nodeSize)
	{
		freeBytes -= size + NODE_HEADER;
		CacheNodePtr rest(node.offset + size + NODE_HEADER);
		rest.setType(FREE_TYPE);
		rest.setHandle(FREE_HANDLE);
		rest.setSizeDiff(0);
		rest.setSize(nodeSize - size - NODE_HEADER);
		rest.blockThreadInsertAfter(node);
		rest.mruThreadReplace(node);
	}
	else
	{
		node.unlinkFromMruThread();
		slop += node.getSize() - size;
		node.setSizeDiff(node.getSize() - size);
		freeBytes -= size + node.getSizeDiff();
		totalFreeBlocks--;
		totalBlocks--;
	}
	if (node.offset == largestFreeNode.offset)
		findLargestFreeNode();
	node.setHandle(handle);
	node.setType(type);
	node.setSize(size);
	node.freeSpaceToBlockMru();
	totalBlocks++;
	totalAllocatedBlocks++;
	allocatedBytes += size;
	data = node.offset + NODE_HEADER;
	if (handle != NO_HANDLE)
		CacheHandler::handles[handle] = data;
	return data;
}

void CacheNodePtr::findLargestFreeNode(void)
{
	largestFreeBytes = 0;
	CacheNodePtr node = freeRoot;
	unsigned count = 0;
	do
	{
		count++;
		if (node.getSize() > largestFreeBytes)
		{
			largestFreeNode = node.offset;
			largestFreeBytes = node.getSize();
		}
		node = node.getLess();
	} while (node.offset != freeRoot.offset && totalFreeBlocks + 1 >= count);
	if (totalFreeBlocks + 1 < count)
		halt(__FILE__, 366);	// __LINE__
}

// Frees a node, merging it with free neighbours. Returns the bytes freed.
long CacheNodePtr::deallocate(CacheNodePtr node)
{
	unsigned handle = node.getHandle();
	if (handlers[node.getType()])
		handlers[node.getType()]->cacheOut(node.offset + NODE_HEADER, handle);
	if (handle < NO_HANDLE)
		CacheHandler::handles[handle] = 0;
	node.unlinkFromMruThread();
	node.setHandle(FREE_HANDLE);
	node.setType(FREE_TYPE);
	long size = node.getSize() + node.getSizeDiff();
	if (size > largestFreeBytes)
	{
		largestFreeBytes = size;
		largestFreeNode = node.offset;
	}
	allocatedBytes -= node.getSize();
	node.setSize(size);
	freeBytes += size;
	slop -= node.getSizeDiff();
	node.setSizeDiff(0);
	totalAllocatedBlocks--;
	totalFreeBlocks++;
	node.addToFreeChain();
	long freed = size;
	CacheNodePtr next = node.getNext();
	if (next.getHandle() == FREE_HANDLE)
	{
		node.merge();
		freed += NODE_HEADER;
	}
	CacheNodePtr prev = node.getPrev();
	if (prev.getHandle() == FREE_HANDLE)
	{
		prev.merge();
		freed += NODE_HEADER;
		return freed;
	}
	return freed;
}

void CacheNodePtr::externalDeallocate(unsigned long data)
{
	if (data)
		deallocate(CacheNodePtr(data - NODE_HEADER));
}

// Frees the oldest nodes until enough room can be collected.
void CacheNodePtr::release(long size)
{
	long limit = totalBytes / 10;
	if (size - freeBytes > limit)
		limit = size - freeBytes;
	long freed = 0;
	CacheNodePtr node = blockRoot.getMore();
	while (freed < limit)
	{
		CacheNodePtr next = node.getMore();
		freed += deallocate(node);
		node = next;
	}
	garbageCollect(size);
}

// Slides allocated nodes towards the start of the cache, gathering the
// free space behind them, until a free node of size bytes exists.
void CacheNodePtr::garbageCollect(long size)
{
	garbageCount++;
	long gap = 0;
	CacheNodePtr node = blockRoot.offset + 2 * NODE_HEADER;
	unsigned char done = 0;
	CacheNodePtr prev;
	while (node.offset != blockRoot.offset)
	{
		if (node.isDummy())
			halt(__FILE__, 482);	// __LINE__
		CacheNodePtr next = node.getNext();
		prev = node.getPrev();
		if (prev.getSizeDiff() > 0)
		{
			unsigned diff = prev.getSizeDiff();
			if (node.isAllocated())
			{
				gap += diff;
				CacheNodePtr dest = node.offset - gap;
				node.shiftUp(gap);
				slop -= diff;
				freeBytes += diff;
				prev.setSizeDiff(0);
			}
			else if (node.isFree())
			{
				if (gap > 0)
				{
					long nodeSize = node.getSize();
					node.shiftFreeBlock(gap);
					if (gap + nodeSize >= size)
					{
						done = 1;
						break;
					}
					node = node.offset - gap;
				}
				CacheNodePtr dest = node.offset - diff;
				node.shiftUp(diff);
				gap = 0;
				slop -= diff;
				dest.setSize(dest.getSize() + diff);
				freeBytes += diff;
				prev.setSizeDiff(0);
				if (dest.getSize() >= size)
				{
					done = 1;
					break;
				}
			}
		}
		else if (node.isAllocated())
		{
			if (prev.isFree())
			{
				gap += prev.getSize() + NODE_HEADER;
				node.shiftUpToFreeBlock();
			}
			else if (gap > 0)
				node.shiftUp(gap);
		}
		else if (node.isFree() && gap > 0)
		{
			long nodeSize = node.getSize();
			node.shiftFreeBlock(gap);
			if (gap + nodeSize >= size)
			{
				done = 1;
				break;
			}
			gap = 0;
		}
		node = next;
	}
	if (!done)
	{
		CacheNodePtr last = blockRoot.getPrev();
		if (gap > 0 || last.getSizeDiff() > 0)
		{
			unsigned diff = last.getSizeDiff();
			CacheNodePtr space = last.offset + last.getSize() + diff + NODE_HEADER;
			if (diff > 0)
			{
				space = space.offset - diff;
				slop -= diff;
				gap += diff;
				last.setSizeDiff(0);
			}
			gap -= NODE_HEADER;
			space.setNext(blockRoot);
			space.setPrev(last);
			blockRoot.setPrev(space);
			last.setNext(space);
			space.setSize(gap);
			space.setSizeDiff(0);
			space.setHandle(FREE_HANDLE);
			space.setType(FREE_TYPE);
			space.addToFreeChain();
			freeBytes = gap;
			totalFreeBlocks = 1;
			totalBlocks++;
		}
	}
	findLargestFreeNode();
}

// Joins the free node after this one onto it.
void CacheNodePtr::merge(void)
{
	CacheNodePtr next = getNext();
	next.unlinkFromMruThread();
	setMergeLink(next);
	setSize(getSize() + next.getSize() + NODE_HEADER);
	freeBytes += NODE_HEADER;
	totalFreeBlocks--;
	totalBlocks--;
	if (largestFreeNode.offset == next.offset || getSize() > largestFreeBytes)
	{
		largestFreeBytes = getSize();
		largestFreeNode = offset;
	}
}

// Moves this node distance bytes towards the start of the cache.
void CacheNodePtr::shiftUp(long distance)
{
	long to = offset - distance;
	unsigned handle = getHandle();
	if (handle < NO_HANDLE)
		CacheHandler::handles[handle] = to + NODE_HEADER;
	reset(CacheNodePtr(to));
	char *dest = (char *)getPtr(to);
	char *src = (char *)getPtr(offset);
	memcpy(dest, src, getSize() + NODE_HEADER);
	int type = ((CacheNode *)dest)->type;
	CacheNodePtr moved;	// never used, but it has a frame slot
	if (type < 0xff)
		handlers[type]->notifyMoved(to + NODE_HEADER);
}

// Moves this node down over the free node in front of it.
void CacheNodePtr::shiftUpToFreeBlock(void)
{
	CacheNodePtr prev = getPrev();
	long to = offset - (prev.getSize() + NODE_HEADER);
	CacheNodePtr before = prev.getPrev();
	unsigned handle = getHandle();
	if (handle < NO_HANDLE)
		CacheHandler::handles[handle] = to + NODE_HEADER;
	shiftUpSet(CacheNodePtr(to));
	char *dest = (char *)getPtr(to);
	char *src = (char *)getPtr(offset);
	memcpy(dest, src, getSize() + getSizeDiff() + NODE_HEADER);
	CacheNodePtr moved(to);
	moved.setPrev(before);
	freeBytes += NODE_HEADER;
	totalFreeBlocks--;
	totalBlocks--;
	int type = ((CacheNode *)dest)->type;
	handlers[type]->notifyMoved(to + NODE_HEADER);
}

// Frees the least recently used node of a type; returns its handle.
int CacheNodePtr::deallocateLru(CacheNodeType type)
{
	CacheNodePtr node = blockRoot;
	unsigned handle = FREE_HANDLE;
	do
	{
		node = node.getMore();
		if (node.isFree())
			break;
		if (node.getType() == type)
		{
			handle = node.getHandle();
			deallocate(node);
			break;
		}
	} while (node.offset != blockRoot.offset);
	return handle;
}

void CacheNodePtr::verifyFailed(unsigned short check, char *file, unsigned short line)
{
	asm {
		mov al, 0x83
		mov ah, 0
		int 0x10
	}
	textmode(C80);
	textmode(C4350);
	traceGet((TraceLevel)50, "verify failed at %d in %s, line %d\r\n", check, file, line);
	dumpCache(0);
	if (haltOnFail)
		halt(file, line);
}

// Walks every thread of the cache and checks its totals. The first
// argument of each verifyFailed call is the line of the failed check.
void CacheNodePtr::verify(char *file, unsigned short line)
{
	HoldPointers hold;
	if (!initialized)
		return;
	if (largestFreeNode.getHandle() != FREE_HANDLE && freeBytes != 0)
		verifyFailed(979, file, line);
	if (totalAllocatedBlocks + totalFreeBlocks != totalBlocks)
		verifyFailed(983, file, line);
	if (totalBytes - (totalBlocks * NODE_HEADER + allocatedBytes + slop + NODE_HEADER) != freeBytes)
		verifyFailed(987, file, line);
	CacheNodePtr last = blockRoot.getPrev();
	long lastSize = last.getSize();
	unsigned char lastDiff = last.getSizeDiff();
	if (last.offset + lastSize + lastDiff + NODE_HEADER - blockRoot.offset != totalBytes)
		verifyFailed(995, file, line);

	// Every node, walked forwards and backwards at once.
	long total = 0;
	CacheNodePtr forward = blockRoot, back = blockRoot;
	unsigned i;
	for (i = 0; i < totalBlocks; i++)
	{
		forward = forward.getNext();
		back = back.getPrev();
		total += forward.getSize() + back.getSize() + forward.getSizeDiff() + back.getSizeDiff() + 2 * NODE_HEADER;
		if ((forward.getHandle() == 0xffff || forward.getHandle() == FREE_HANDLE) && forward.getType() != 0xff)
			verifyFailed(1025, file, line);
		int type = forward.getType();
		if (type < 6 && handlers[type] && !handlers[type]->verify(forward.offset))
			verifyFailed(1032, file, line);
	}
	if (forward.offset != blockRoot.offset)
		verifyFailed(1043, file, line);
	if (total >> 1 != totalBytes - NODE_HEADER)
		verifyFailed(1046, file, line);

	// The allocated nodes, oldest and newest first.
	total = 0;
	forward = back = blockRoot;
	for (i = 0; i < totalAllocatedBlocks; i++)
	{
		forward = forward.getLess();
		back = back.getMore();
		total += forward.getSize() + back.getSize();
		if ((forward.getHandle() == 0xffff || forward.getHandle() == FREE_HANDLE) && forward.getType() != 0xff)
			verifyFailed(1057, file, line);
	}
	if (forward.offset != blockRoot.offset || back.offset != blockRoot.offset)
		verifyFailed(1063, file, line);
	if (total >> 1 != allocatedBytes)
		verifyFailed(1066, file, line);

	// The free chain.
	total = 0;
	forward = back = freeRoot;
	for (i = 0; i < totalFreeBlocks + 1; i++)
	{
		forward = forward.getLess();
		back = back.getMore();
		total += forward.getSize() + back.getSize();
		if (forward.getHandle() == 0xffff || forward.getHandle() == FREE_HANDLE)
		{
			if (forward.getType() != 0xff)
				verifyFailed(1077, file, line);
		}
		else
			verifyFailed(1081, file, line);
	}
	if (forward.offset != freeRoot.offset || back.offset != freeRoot.offset)
		verifyFailed(1086, file, line);
	if (total >> 1 != freeBytes)
		verifyFailed(1089, file, line);

	// Every handle in use points at a node.
	for (int h = 0; h < MAX_HANDLES; h++)
	{
		long data = CacheHandler::handles[h];
		if (data)
		{
			data -= NODE_HEADER;
			CacheNodePtr node = blockRoot;
			unsigned char found = 0;
			unsigned n = 0;
			while (!found && n < totalBlocks)
			{
				node = node.getNext();
				if (node.offset == data)
					found++;
				n++;
			}
			if (!found)
				verifyFailed(1108, file, line);
		}
	}
}

void CacheNodePtr::dumpCache(unsigned char)
{
	HoldPointers hold;
	printf("total = %ld allocated = %ld free = %ld slop = %ld\r\n", totalBytes, allocatedBytes, freeBytes, slop);
	printf("total blocks = %d allocated blocks = %d free blocks = %d largest bytes = %ld", totalBlocks, totalAllocatedBlocks, totalFreeBlocks, largestFreeBytes);
	printf("\r\n         block  size   diff next   prev   type handle \r\n");
	printf("         ------ ------ ---- ------ ------ ---- ------ \r\n");
	CacheNodePtr node = blockRoot;
	for (unsigned i = 0; i < totalBlocks; i++)
	{
		node.dump();
		node = node.getNext();
	}
	freeRoot.dump();
}

void CacheNodePtr::dump(void)
{
	char *blockMark;
	if (offset == blockRoot.offset)
		blockMark = "BR";
	else
		blockMark = "  ";
	char *freeMark;
	if (offset == freeRoot.offset)
		freeMark = "FR";
	else
		freeMark = "  ";
	char *largestMark;
	if (offset == largestFreeNode.offset)
		largestMark = "LF";
	else
		largestMark = "  ";
	char handle[16];
	if (getHandle() == FREE_HANDLE)
		strcpy(handle, "FREE");
	else if (getHandle() == 0xffff)
		strcpy(handle, "DUMMY");
	else
		sprintf(handle, "%0X", getHandle());
	unsigned shape = 0;
	unsigned frame = 0;
	int type = getType();
	if (type == 4)
	{
		FrameData *data = (FrameData *)getPtr(offset + NODE_HEADER);
		shape = data->shape;
		frame = data->frame;
	}
	printf("%2s %2s %2s %06lX %06lX %04x %06lX %06lX %02X %04X %04X %02X %-5s", blockMark, freeMark, largestMark,
		offset, getSize(), getSizeDiff(), getNext(), getPrev(), getType(), getHandle(), shape, frame, handle);
	printf("\r\n");
}

void CacheNodePtr::profileCache(void)
{
	charGen.makeString(1, 1, "%d pct full\n%d items in cache\n%d total free blocks\n%d number of garbage collects",
		getPercentFull(), totalAllocatedBlocks, totalFreeBlocks, garbageCount);
}

int CacheNodePtr::getPercentFull(void)
{
	long used = allocatedBytes;
	long total = totalBytes;
	long percent = used * 100 / total;
	return percent;
}

// Writes the cache image with its links made relative to the cache start,
// then puts the live image back.
void CacheNodePtr::save(char *name)
{
	char *image = new char[totalBytes];
	if (image == 0)
		outOfMemory(__FILE__, 1207);	// __LINE__
	memcpy(image, getPtr(cache.offset), totalBytes);
	FILE *file = fopen(name, "wb");
	if (file == 0)
		unableToOpenFile(__FILE__, 1214, name);	// __LINE__
	fwrite(&totalBytes, 4, 1, file);
	fwrite(&allocatedBytes, 4, 1, file);
	fwrite(&freeBytes, 4, 1, file);
	fwrite(&largestFreeBytes, 4, 1, file);
	fwrite(&slop, 4, 1, file);
	fwrite(&totalBlocks, 2, 1, file);
	fwrite(&totalAllocatedBlocks, 2, 1, file);
	fwrite(&totalFreeBlocks, 2, 1, file);
	CacheNodePtr node = blockRoot;
	do
	{
		CacheNode *header = (CacheNode *)getPtr(node.offset);
		CacheNodePtr next = header->next;
		header->next -= cache.offset;
		header->prev -= cache.offset;
		header->less -= cache.offset;
		header->more -= cache.offset;
		node = next;
	} while (node.offset != blockRoot.offset);
	CacheNode *header = (CacheNode *)getPtr(freeRoot.offset);
	header->next -= cache.offset;
	header->prev -= cache.offset;
	header->less -= cache.offset;
	header->more -= cache.offset;
	fwrite(getPtr(cache.offset), totalBytes, 1, file);
	long relative = blockRoot.offset - cache.offset;
	fwrite(&relative, 4, 1, file);
	relative = freeRoot.offset - cache.offset;
	fwrite(&relative, 4, 1, file);
	relative = largestFreeNode.offset - cache.offset;
	fwrite(&relative, 4, 1, file);
	fwrite(&CacheHandler::numHandles, 2, 1, file);
	for (int h = 0; h < CacheHandler::numHandles; h++)
	{
		long data;
		if (CacheHandler::handles[h])
			data = CacheHandler::handles[h] - cache.offset;
		else
			data = 0;
		fwrite(&data, 4, 1, file);
	}
	memcpy(getPtr(cache.offset), image, totalBytes);
	delete image;
	fclose(file);
}

void CacheNodePtr::load(char *name)
{
	FILE *file = fopen(name, "rb");
	if (file == 0)
		unableToOpenFile(__FILE__, 1274, name);	// __LINE__
	fread(&totalBytes, 4, 1, file);
	fread(&allocatedBytes, 4, 1, file);
	fread(&freeBytes, 4, 1, file);
	fread(&largestFreeBytes, 4, 1, file);
	fread(&slop, 4, 1, file);
	fread(&totalBlocks, 2, 1, file);
	fread(&totalAllocatedBlocks, 2, 1, file);
	fread(&totalFreeBlocks, 2, 1, file);
	fread(getPtr(cache.offset), totalBytes, 1, file);
	long relative;
	fread(&relative, 4, 1, file);
	blockRoot.offset = cache.offset + relative;
	fread(&relative, 4, 1, file);
	freeRoot.offset = cache.offset + relative;
	fread(&relative, 4, 1, file);
	largestFreeNode.offset = cache.offset + relative;
	fread(&CacheHandler::numHandles, 2, 1, file);
	for (int h = 0; h < CacheHandler::numHandles; h++)
	{
		long data;
		fread(&data, 4, 1, file);
		if (data)
			CacheHandler::handles[h] = cache.offset + data;
		else
			CacheHandler::handles[h] = 0;
	}
	CacheNodePtr node = blockRoot;
	do
	{
		CacheNode *header = (CacheNode *)getPtr(node.offset);
		header->next = cache.offset + header->next;
		header->prev = cache.offset + header->prev;
		header->less = cache.offset + header->less;
		header->more = cache.offset + header->more;
		node = header->next;
	} while (node.offset != blockRoot.offset);
	CacheNode *header = (CacheNode *)getPtr(freeRoot.offset);
	header->next = cache.offset + header->next;
	header->prev = cache.offset + header->prev;
	header->less = cache.offset + header->less;
	header->more = cache.offset + header->more;
	fclose(file);
}
