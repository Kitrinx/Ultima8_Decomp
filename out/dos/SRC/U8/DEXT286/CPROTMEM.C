// flags: -P -3 -O -Ob -Oe -Og -Oi -Om -Ov -Z -G -k- -d
// name: DEXT286\CPROTMEM.C

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <phapi.h>
#include "CEXIT.H"
#include "INIT.H"
#include "APPDEBUG.H"
#include "CDESC.H"
#include "SELMNGR.H"
#include "CPROTMEM.H"

extern "C" void superMemCopy(unsigned long dest, unsigned long src, unsigned long count);

#define MIN_BLOCK 16L
#define PAGE_SIZE 0x1000L

unsigned FRAGMENT_0_PERCENTAGE = 45;
unsigned char ProtMemoryManager::initialized = 0;
unsigned char ProtMemoryManager::fromLargest = 0;
unsigned char ProtMemoryManager::failedLastRequest = 0;
unsigned char newAddress1[4];
unsigned char newAddress2[4];
Fragment ProtMemoryManager::fragments[MAX_FRAGMENTS];
unsigned short ProtMemoryManager::numFragments;
unsigned char reserved[1];
unsigned char addr_xhi[1];

void *malloc(size_t size)
{
	if (!ProtMemoryManager::initialized)
		ProtMemoryManager::initialize();
	return ProtMemoryManager::allocate(size, 0, 1);
}

void far *farmalloc(unsigned long size)
{
	if (!ProtMemoryManager::initialized)
		ProtMemoryManager::initialize();
	return ProtMemoryManager::allocate(size, 0, 1);
}

void farfree(void far *block)
{
	if (block == 0)
		halt(__FILE__, 267);	// __LINE__
	ProtMemoryManager::free(block, 0);
}

void free(void *block)
{
	if (block == 0)
		halt(__FILE__, 278);	// __LINE__
	ProtMemoryManager::free(block, 0);
}

int heapcheck(void)
{
	return _HEAPOK;
}

int farheapcheck(void)
{
	return _HEAPOK;
}

unsigned short FreeDescriptor::getIndex(void)
{
	unsigned short index;

	index = indexHigh << 8;
	index |= indexLow;
	return index;
}

void FreeDescriptor::setIndex(unsigned short index)
{
	indexHigh = index >> 8;
	indexLow = index & 0xFF;
}

unsigned long FreeBlock::getNext(void)
{
	unsigned long a;

	a = LocalDescriptorTable::getDesc(FP_SEG(this))->getBase();
	a += size + 1;
	return a;
}

// First fit; returns the linear address of the data, or 0.
unsigned long Fragment::allocate(unsigned long size, unsigned short tag)
{
	FreeBlock *head;
	unsigned long cur;
	FreeBlock *b;
	unsigned long avail;

	if (size > 0x100000L && (size & 0xFFF)) {
		size &= 0xFFFFF000L;
		size += PAGE_SIZE;
	} else if (size & 1)
		size++;
	size += BLOCK_HEADER;
	if (size < MIN_BLOCK)
		size = MIN_BLOCK;
	head = (FreeBlock *)makePtr(start);
	cur = head->next;
	do {
		b = (FreeBlock *)makePtr(cur);
		avail = b->getLength();
		if (!b->isEnd() && avail >= size) {
			FreeBlock *prev = (FreeBlock *)makePtr(b->prev);
			FreeBlock *next = (FreeBlock *)makePtr(b->next);
			prev->setNext(b->next);
			next->setPrev(b->prev);
			used++;
			if (avail - size >= MIN_BLOCK) {
				unsigned long rest = cur + size;
				FreeBlock *after = (FreeBlock *)makePtr(cur + b->getLength());
				after->setPrevBlock(rest);
				FreeBlock *r = (FreeBlock *)makePtr(rest);
				r->setFreeLength(avail - size);
				r->setPrevBlock(cur);
				FreeBlock *h = (FreeBlock *)makePtr(start);
				FreeBlock *hn = (FreeBlock *)makePtr(h->next);
				r->setNext(h->next);
				r->setPrev(start);
				h->setNext(rest);
				hn->setPrev(rest);
			} else {
				size = avail;
				freeBlocks--;
			}
			b->setSize(size);
			b->setTag(tag);
			return cur + BLOCK_HEADER;
		}
		cur = b->next;
	} while (start != cur);
	return 0;
}

// Frees the block behind selector block, merging it with free neighbours.
void Fragment::free(void *block, unsigned short tag)
{
	unsigned long addr = LocalDescriptorTable::getDesc(FP_SEG(block))->getBase() - BLOCK_HEADER;
	FreeBlock *used = (FreeBlock *)makePtr(addr);
	FreeBlock *b = used;
	unsigned long prevAddr = used->prevBlock;
	FreeBlock *prevBlk = (FreeBlock *)makePtr(prevAddr);
	unsigned long nextAddr = addr + used->getSize();
	FreeBlock *nextBlk = (FreeBlock *)makePtr(nextAddr);

	if (used->tag != tag)
		halt(__FILE__, 497);	// __LINE__
	if (prevBlk->isFree() && start != prevAddr) {
		prevBlk->setFreeLength(prevBlk->size + used->getSize() + 1);
		this->used--;
		if (nextBlk->isFree() && !nextBlk->isEnd()) {
			prevBlk->setFreeLength(prevBlk->getLength() + nextBlk->getLength());
			FreeBlock *p = (FreeBlock *)makePtr(nextBlk->prev);
			FreeBlock *n = (FreeBlock *)makePtr(nextBlk->next);
			p->setNext(nextBlk->next);
			n->setPrev(nextBlk->prev);
			FreeBlock *after = (FreeBlock *)makePtr(nextAddr + nextBlk->getLength());
			after->setPrevBlock(prevAddr);
			freeBlocks--;
		} else
			nextBlk->setPrevBlock(prevAddr);
	} else {
		freeBlocks++;
		this->used--;
		unsigned long size = used->getSize();
		if (nextBlk->isFree() && !nextBlk->isEnd()) {
			b->setFreeLength(nextBlk->size + size + 1);
			FreeBlock *p = (FreeBlock *)makePtr(nextBlk->prev);
			FreeBlock *n = (FreeBlock *)makePtr(nextBlk->next);
			p->setNext(nextBlk->next);
			n->setPrev(nextBlk->prev);
			FreeBlock *after = (FreeBlock *)makePtr(nextAddr + nextBlk->getLength());
			after->setPrevBlock(addr);
			freeBlocks--;
		} else
			b->setFreeLength(size);
		b->setPrevBlock(prevAddr);
		FreeBlock *h = (FreeBlock *)makePtr(start);
		FreeBlock *hn = (FreeBlock *)makePtr(h->next);
		b->setNext(h->next);
		b->setPrev(start);
		h->setNext(addr);
		hn->setPrev(addr);
	}
}

// Slides used blocks down over free ones, leaving one free block at the end.
void Fragment::garbageCollect(void)
{
	FreeBlock *head;
	FreeBlock *f;
	unsigned long src;
	unsigned long dest;
	unsigned long prev;
	unsigned long step;
	FreeBlock *b;
	unsigned long freed;
	unsigned long len;

	head = (FreeBlock *)makePtr(start);
	dest = src = head->getNext();
	b = (FreeBlock *)makePtr(src);
	freed = 0;
	while (b->isFree() && !b->isEnd()) {
		len = b->getLength();
		src += len;
		freed += len;
		b = (FreeBlock *)makePtr(src);
	}
	if (!b->isFree()) {
		prev = start;
		while (!b->isEnd()) {
			if (b->isFree()) {
				len = b->getLength();
				freed += len;
				step = len;
			} else {
				step = b->getSize();
				if (dest != src) {
					asm pushf
					asm cli
					superMemCopy(dest, src, step);
					SelectorManager::remapBase(src + BLOCK_HEADER, dest + BLOCK_HEADER);
					asm popf
					b = (FreeBlock *)makePtr(dest);
					b->setPrevBlock(prev);
				}
				prev = dest;
				dest += step;
			}
			src += step;
			b = (FreeBlock *)makePtr(src);
			if (b->isEnd() && freed) {
				f = (FreeBlock *)makePtr(dest);
				f->setFreeLength(freed);
				f->setPrevBlock(prev);
				f->setPrev(start);
				f->setNext(src);
				freeBlocks = 3;
				head = (FreeBlock *)makePtr(start);
				head->setNext(dest);
				b->setPrevBlock(dest);
				b->setPrev(dest);
			}
		}
	}
}

// Walks the free list both ways and the blocks both ways, halting on a broken link.
void Fragment::verify(char *, int)
{
	unsigned long addr;
	FreeBlock *b;
	unsigned short blocks = freeBlocks + used;
	unsigned short i;

	for (i = 0, addr = start; i < freeBlocks; i++) {
		b = (FreeBlock *)makePtr(addr);
		addr = b->next;
	}
	if (start != addr)
		halt(__FILE__, 803);	// __LINE__
	for (i = 0, addr = start; i < freeBlocks; i++) {
		b = (FreeBlock *)makePtr(addr);
		addr = b->prev;
	}
	if (start != addr)
		halt(__FILE__, 823);	// __LINE__
	for (i = 0, addr = start; i < blocks; i++) {
		b = (FreeBlock *)makePtr(addr);
		if (b->isFree())
			addr = b->prevBlock;
		else
			addr = b->prevBlock;
	}
	if (start != addr)
		halt(__FILE__, 846);	// __LINE__
	for (i = 0, addr = start; i < blocks; i++) {
		b = (FreeBlock *)makePtr(addr);
		if (b->isFree()) {
			if (b->isEnd())
				addr += MIN_BLOCK;
			else
				addr += b->getLength();
		} else
			addr += b->getSize();
	}
	if (start + length != addr)
		halt(__FILE__, 874);	// __LINE__
}

void Fragment::getMemStatus(unsigned long &total, unsigned long &largest)
{
	unsigned long size;
	FreeBlock *head;
	unsigned long cur;
	FreeBlock *b;

	total = largest = 0;
	if (start) {
		head = (FreeBlock *)makePtr(start);
		cur = head->next;
		for (;;) {
			b = (FreeBlock *)makePtr(cur);
			if (b->isEnd())
				break;
			size = b->size + 1 - BLOCK_HEADER;
			total += size;
			if (largest < size)
				largest = size;
			cur = b->next;
		}
	}
}

void ProtMemoryManager::verify(char *file, int line)
{
	unsigned short i;

	for (i = 0; i < numFragments; i++)
		fragments[i].verify(file, line);
}

// Tries each fragment, then twice more after collecting garbage.
void *ProtMemoryManager::allocate(unsigned long size, unsigned short tag, unsigned char anyFragment)
{
	int last;
	int step;
	int pass;
	unsigned short sel;
	int i;

	if (size == 0) {
		failedLastRequest = 1;
		return 0;
	}
	if (size & 1) {
		if (size > 0x0FFFFFF4L) {
			failedLastRequest = 1;
			return 0;
		}
	} else if (size > 0x0FFFFFF5L) {
		failedLastRequest = 1;
		return 0;
	}
	sel = SelectorManager::allocateSelector();
	if (sel == 0) {
		failedLastRequest = 1;
		return 0;
	}
	for (pass = 0; pass < 3; pass++) {
		if (anyFragment) {
			if (fromLargest) {
				i = 1;
				last = numFragments - 1;
				step = 1;
			} else {
				i = numFragments - 1;
				last = 1;
				step = -1;
			}
		} else {
			i = 0;
			last = 0;
			step = 1;
		}
		for (; fromLargest ? i <= last : i >= 0; i += step) {
			unsigned long addr = fragments[i].allocate(size, tag);

			if (addr) {
				SelectorManager::setupDescriptor(sel, addr, size);
				failedLastRequest = 0;
				return MK_FP(sel, 0);
			}
		}
		if (pass < 2)
			garbageCollect(pass);
	}
	SelectorManager::deallocateSelector(sel);
	failedLastRequest = 1;
	return 0;
}

void ProtMemoryManager::garbageCollect(unsigned char level)
{
	unsigned short i;

	applicationGarbageCollect(level);
	for (i = 1; i < numFragments; i++)
		fragments[i].garbageCollect();
}

void ProtMemoryManager::free(void *block, unsigned short tag)
{
	Desc *desc;
	unsigned long base;
	unsigned short i;

	if (FP_OFF(block) != 0)
		halt(__FILE__, 1099);	// __LINE__
	desc = LocalDescriptorTable::getDesc(FP_SEG(block));
	if (desc->access != 0xF3)
		halt(__FILE__, 1104);	// __LINE__
	base = desc->getBase();
	for (i = 0; i < numFragments; i++)
		if (fragments[i].contains(base)) {
			fragments[i].free(block, tag);
			break;
		}
	if (i == numFragments)
		halt(__FILE__, 1124);	// __LINE__
	SelectorManager::deallocateSelector(FP_SEG(block));
}

// Sizes extended memory, takes FRAGMENT_0_PERCENTAGE of it as fragment 0 and
// the rest, up to 12M, as further fragments. Conventional memory is held
// meanwhile so the extender cannot hand it out.
void ProtMemoryManager::allocateFragments(void)
{
	unsigned short realSel;
	unsigned short realPara;
	unsigned short extraSel;
	unsigned short extraPara;
	unsigned long realSize = 0x16000L;
	unsigned long realExtra = 0;
	long size;
	long smaller;
	Fragment *frag;
	unsigned char got;
	unsigned char canShrink;
	unsigned long total;
	unsigned long addr;
	unsigned long lastFail;
	unsigned long frag0Size;
	long nextSize;
	unsigned long first;
	unsigned long last;
	FreeBlock *head;
	FreeBlock *firstBlk;
	FreeBlock *endBlk;

	if (realSize > 0x10000L) {
		realExtra = realSize - 0x10000L;
		realSize = 0x16000L - realExtra;
		if (DosAllocRealSeg(realExtra, &extraPara, &extraSel))
			halt(__FILE__, 1308);	// __LINE__
	}
	if (DosAllocRealSeg(realSize, &realPara, &realSel))
		halt(__FILE__, 1312);	// __LINE__

	// Find the total by grabbing the largest blocks the extender will give.
	{
		unsigned long blocks[MAX_FRAGMENTS];
		unsigned short i;

		canShrink = 1;
		numFragments = 0;
		total = 0;
		lastFail = 0;
		for (size = 0x800000L; size > 0; size -= 0x400) {
			got = !DosAllocLinMem(size, &addr);
			if (got) {
				total += size;
				blocks[numFragments] = addr;
				numFragments++;
				canShrink = 1;
				lastFail = 0;
			} else if (canShrink && lastFail > size && size - 0x40000L > 0) {
				size -= 0x40000L;
				got = !DosAllocLinMem(size, &addr);
				if (got) {
					DosFreeLinMem(addr);
					lastFail = size;
					canShrink = 0;
					size += 0x40000L;
				}
			}
		}
		for (i = 0; i < numFragments; i++)
			DosFreeLinMem(blocks[i]);
	}

	if (frag0Size > 0x100000L)
		frag0Size = 0x100000L;
	frag0Size = FRAGMENT_0_PERCENTAGE * total / 100;
	if (frag0Size < 0x80000L) {
		failedLastRequest = 0;
		printf("Fragment 0 below minimum size.  Try the '-d %d' option.\n\r", 100 - FRAGMENT_0_PERCENTAGE - 5);
		halt(__FILE__, 1404);	// __LINE__
	}
	frag0Size &= 0xFFFFFFFCL;
	got = !DosAllocLinMem(frag0Size, &addr);
	if (!got)
		while (!got && frag0Size >= 0x80000L) {
			got = !DosAllocLinMem(frag0Size, &addr);
			frag0Size -= 0x2000;
		}
	if (!got)
		halt(__FILE__, 1418);	// __LINE__
	if (addr == 0)
		halt(__FILE__, 1427, "Could not allocate fragment 0 at a size of %ld bytes.\n\r", frag0Size);	// __LINE__

	got = 0;
	total = 0;
	canShrink = 1;
	nextSize = 0x800000L;
	for (numFragments = 0; numFragments < MAX_FRAGMENTS && total < 0xC00000L; numFragments++) {
		if (size < 0)
			break;
		frag = &fragments[numFragments];
		if (numFragments == 0) {
			size = frag0Size;
			nextSize = 0x800000L;
			frag->start = addr;
			got = 1;
		} else {
			size = nextSize;
			while (size > 0 && !got) {
				got = !DosAllocLinMem(size, &frag->start);
				if (got)
					canShrink = 1;
				else {
					size -= 0x400;
					if (canShrink) {
						smaller = size - 0x40000L;
						if (smaller > 0) {
							unsigned char fits = !DosAllocLinMem(smaller, &frag->start);

							if (fits) {
								canShrink = 0;
								DosFreeLinMem(frag->start);
							} else
								size = smaller;
						} else
							canShrink = 0;
					}
				}
			}
		}
		if (numFragments) {
			if (size > 0)
				nextSize = size;
			if (got == 0)
				break;
			total += size;
		}
		got = 0;
		fragments[numFragments].length = size;
		fragments[numFragments].used = 0;
		fragments[numFragments].freeBlocks = 3;

		// A 16-byte head block, one free block, and an end marker, in a ring.
		first = frag->start + MIN_BLOCK;
		last = frag->start + size - MIN_BLOCK;
		head = (FreeBlock *)makePtr(frag->start);
		firstBlk = (FreeBlock *)makePtr(first);
		endBlk = (FreeBlock *)makePtr(last);
		head->size = MIN_BLOCK - 1;
		head->setPrevBlock(frag->start + size - MIN_BLOCK);
		head->setPrev(frag->start + size - MIN_BLOCK);
		head->setNext(first);
		firstBlk->setFreeLength(size - 32);
		firstBlk->setPrevBlock(frag->start);
		firstBlk->setPrev(frag->start);
		firstBlk->setNext(frag->start + size - MIN_BLOCK);
		endBlk->size = 0xFFFFFFFFL;
		endBlk->setPrevBlock(first);
		endBlk->setPrev(first);
		endBlk->setNext(frag->start);
	}
	if (realExtra && DosFreeSeg(extraSel))
		Fatal("%s %d", __FILE__, 1559);	// __LINE__
	if (DosFreeSeg(realSel))
		Fatal("%s %d", __FILE__, 1563);	// __LINE__
}

void ProtMemoryManager::freeFragments(void)
{
	unsigned short i;

	for (i = 0; i < numFragments; i++)
		DosFreeLinMem(fragments[i].start);
}

void ProtMemoryManager::getMemStatus(short i, unsigned long &total, unsigned long &largest)
{
	if (i >= numFragments)
		halt(__FILE__, 1579);	// __LINE__
	fragments[i].getMemStatus(total, largest);
}

void ProtMemoryManager::getMemStatus(unsigned long &total, unsigned long &largest, unsigned short &selectors)
{
	unsigned short i;
	unsigned long fragTotal;
	unsigned long fragLargest;

	total = largest = 0;
	for (i = 0; i < numFragments; i++) {
		fragments[i].getMemStatus(fragTotal, fragLargest);
		if (largest < fragLargest)
			largest = fragLargest;
		total += fragTotal;
	}
	selectors = SelectorManager::currentSelectorCount;
}

void pascal CLEANUP(unsigned short)
{
	SimpleExitSystem::exit();
	ProtMemoryManager::freeFragments();
	SelectorManager::uninit();
	DosExitList(EXLST_EXIT, 0);
}

// Reads the -dNN (dynamic memory percentage) option straight from the PSP.
void ProtMemoryManager::initialize(void)
{
	int i = 0;
	unsigned short psp;
	char *cmd;
	char number[4];

	asm {
		mov ah, 62h
		int 21h
		mov psp, bx
	}
	cmd = (char *)MK_FP(psp, 0x80);
	if (*cmd) {
		if (cmd[2] == '-')
			i = 2;
		else if (cmd[3] == '-')
			i = 3;
		if (i && (*(cmd + i + 1) == 'd' || *(cmd + i + 1) == 'D')) {
			long digits = *(long *)(cmd + i + 2);
			*(long *)number = digits;
			number[3] = 0;
			FRAGMENT_0_PERCENTAGE = 100 - atol(number);
			printf("\n\r");
			printf("Ultima VIII MMU:  ");
			if (FRAGMENT_0_PERCENTAGE < 5 || FRAGMENT_0_PERCENTAGE > 95) {
				printf("valid dynamic memory ranges are 5-95%.\r\n");
				exit(0);
			}
			printf("dynamic memory = %u%%.\r\n", 100 - FRAGMENT_0_PERCENTAGE);
		}
	}
	initReservedSelectors();
	allocateFragments();
	SelectorManager::init();
	initialized = 1;
	DosExitList(EXLST_ADD, CLEANUP);
}
