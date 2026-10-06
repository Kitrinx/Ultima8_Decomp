// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\XCACHE\FLEXHAND.C

#include "CEXIT.H"
#include "FLEXHAND.H"

inline Index::Index(void) { size = 0; offset = 0; }

FlexShapeHandler::FlexShapeHandler(int numHandles, CacheNodeType type, const char *name) :
	CacheHandler(type)
{
	startHandle = reserve(numHandles);
	CacheNodePtr::attach(this, type);
	flexFile = new FlexFile((char *)name, (OpenMode)0, -1);
	if (flexFile == 0)
		outOfMemory(__FILE__, 16);	// __LINE__
}

FlexShapeHandler::~FlexShapeHandler(void)
{
	delete flexFile;
}

char *FlexShapeHandler::get(unsigned handle)
{
	unsigned long offset = handles[handle];
	if (offset)
		CacheNodePtr(offset - NODE_HEADER).makeMru();
	else
	{
		Index index;
		int record = handle - startHandle;
		flexFile->getIndex(record, index);
		if (index.size == 0)
			halt(__FILE__, 45);	// __LINE__
		long node = CacheNodePtr::allocate(index.size, handle, (CacheNodeType)1);
		void *data = CacheNodePtr::getPtr(node);
		flexFile->readRecord(record, data, -1, 0);
	}
	return (char *)CacheNodePtr::getPtr(handles[handle]);
}
