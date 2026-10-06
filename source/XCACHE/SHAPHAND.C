// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\XCACHE\SHAPHAND.C

#include <mem.h>
#include "CEXIT.H"
#include "CRECT.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "SHAPHAND.H"

#define NUM_SHAPE_HANDLES	2048

unsigned ShapeHandler::startHandle;
ShapeFlex *ShapeHandler::shapeFile;
ShapeHandler *shapeHandler;
FrameHandler *frameHandler;

inline Index::Index(void) { size = 0; offset = 0; }
inline int isEmpty(Index &index) { return index.size == 0; }

ShapeFlex::ShapeFlex(void) :
	FastFlex(fileSpec(0, StaticDir, shapeFileName, 0), 0, (OpenMode)0, -1)
{
}

ShapeHandler::ShapeHandler(void) :
	CacheHandler((CacheNodeType)0)
{
	startHandle = reserve(NUM_SHAPE_HANDLES);
	CacheNodePtr::attach(this, (CacheNodeType)0);
	shapeFile = new ShapeFlex;
	if (shapeFile == 0)
		outOfMemory(__FILE__, 490);	// __LINE__
}

void ShapeHandler::getDim(unsigned short shape, unsigned short frame, int &width, int &height, int &xoff, int &yoff)
{
	FrameHeader *header = (FrameHeader *)get(shape, frame);
	if (header == 0)
	{
		trace((TraceLevel)50, "nullShape t%d f%d!\n\r", shape, frame);
		header = (FrameHeader *)get(0, 0);
	}
	width = header->width;
	height = header->height;
	xoff = header->xoff;
	yoff = header->yoff;
}

void ShapeHandler::getInfo(unsigned short shape, int &maxWidth, int &maxHeight, int &numFrames)
{
	if (handles[shape] == 0)
	{
		char *data = get(shape, 0);
		if (data == 0)
		{
			trace((TraceLevel)50, "nullShape t%d f0!\n\r", shape);
			shape = 0;
			get(0, 0);
		}
	}
	ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(handles[shape]);
	maxWidth = index->maxWidth;
	maxHeight = index->maxHeight;
	numFrames = index->numFrames;
}

void ShapeHandler::getRect(unsigned short shape, unsigned short frame, Rect &rect, int x, int y)
{
	int width, height, xoff, yoff;
	getDim(shape, frame, width, height, xoff, yoff);
	rect.f_00 = x - xoff;
	rect.f_02 = y - yoff;
	rect.f_04 = rect.f_00 + width - 1;
	rect.f_06 = rect.f_02 + height - 1;
}

// Reads one frame into the cache and points the shape's frame table at it.
char *ShapeHandler::loadFrame(unsigned short shape, unsigned short frame, unsigned long shapeOffset, unsigned long frameOffset)
{
	ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(shapeOffset);
	long size = index->frames[frame].size;
	if (size == 0)
	{
		index->frames[frame].offset = 0;
		return 0;
	}
	CacheNodePtr(handles[shape] - NODE_HEADER).makeMru();
	unsigned long node = CacheNodePtr::allocate(size, NO_HANDLE, (CacheNodeType)4);
	if (node == 0 || handles[shape] == 0)
		halt(__FILE__, 573);	// __LINE__
	CacheNodePtr(node - NODE_HEADER).makeMru();
	CacheNodePtr(handles[shape] - NODE_HEADER).makeMru();
	Index fileIndex;
	shapeFile->getIndex(shape, fileIndex);
	long diskOffset = frameOffset & ~FRAME_ON_DISK;
	char *data = (char *)CacheNodePtr::getPtr(node);
	shapeFile->readRecord(shape, data, size, diskOffset);
	index = (ShapeIndex *)CacheNodePtr::getPtr(handles[shape]);
	((FrameData *)data)->oldOffset = index->frames[frame].offset;
	index->frames[frame].offset = node;
	((FrameData *)data)->shape = shape;
	((FrameData *)data)->frame = frame;
	return data;
}

// Returns a frame's header, loading the shape's frame table and the
// frame itself as needed.
char *ShapeHandler::get(unsigned short shape, unsigned short frame)
{
	unsigned long shapeOffset = handles[shape];
	if (shapeOffset)
	{
		ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(shapeOffset);
		if (index->numFrames <= frame)
			return 0;
		unsigned long frameOffset = index->frames[frame].offset;
		if (frameOffset == 0)
			return 0;
		frameOffset &= ~FRAME_FLAG;
		if (frameOffset & FRAME_ON_DISK)
		{
			char *data = loadFrame(shape, frame, shapeOffset, frameOffset);
			if (data)
				data += sizeof(FrameData);
			return data;
		}
		if (frameOffset == 0 || shapeOffset == 0)
			halt(__FILE__, 631);	// __LINE__
		CacheNodePtr(frameOffset - NODE_HEADER).makeMru();
		CacheNodePtr(shapeOffset - NODE_HEADER).makeMru();
		return (char *)CacheNodePtr::getPtr(frameOffset + sizeof(FrameData));
	}
	else
	{
		Index fileIndex;
		shapeFile->getIndex(shape, fileIndex);
		if (isEmpty(fileIndex))
			return 0;
		ShapeHeader header;
		shapeFile->readRecord(shape, &header, sizeof(ShapeHeader), 0);
		int size = header.numFrames * sizeof(FrameSlot) + sizeof(ShapeHeader);
		long node = CacheNodePtr::allocate(size, shape, (CacheNodeType)0);
		ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(node);
		memcpy(index, &header, sizeof(ShapeHeader));
		size -= sizeof(ShapeHeader);
		shapeFile->readRecord(shape, index->frames, size, sizeof(ShapeHeader));
		FrameSlot *entry = index->frames;
		for (unsigned i = 0; i < header.numFrames; i++)
			(entry++)->offset |= FRAME_ON_DISK;
		if (index->numFrames <= frame)
			return 0;
		unsigned long frameOffset = index->frames[frame].offset;
		char *data = loadFrame(shape, frame, node, frameOffset);
		if (data)
			data += sizeof(FrameData);
		return data;
	}
}

// Returns a frame's header and its size on disk.
char *ShapeHandler::get(unsigned short shape, unsigned short frame, unsigned short &size)
{
	size = 0;
	char *data = get(shape, frame);
	if (data == 0)
		return 0;
	ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(handles[shape]);
	size = index->frames[frame].size;
	return data;
}

void ShapeHandler::cacheOut(unsigned long, unsigned)
{
}

// Checks that every loaded frame of the shape at this node names the
// shape and frame it belongs to.
unsigned char ShapeHandler::verify(unsigned long nodeOffset)
{
	CacheNode *node = (CacheNode *)CacheNodePtr::getPtr(nodeOffset);
	unsigned shape = node->handle;
	ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(nodeOffset + NODE_HEADER);
	unsigned numFrames = index->numFrames;
	for (unsigned i = 0; i < numFrames; i++)
	{
		index = (ShapeIndex *)CacheNodePtr::getPtr(nodeOffset + NODE_HEADER);
		unsigned long frameOffset = index->frames[i].offset;
		frameOffset &= ~FRAME_FLAG;
		if (!(frameOffset & FRAME_ON_DISK))
		{
			FrameData *data = (FrameData *)CacheNodePtr::getPtr(frameOffset);
			if (data->shape != shape || data->frame != i)
				return 0;
		}
	}
	return 1;
}

void ShapeHandler::flushType(unsigned short shape)
{
	unsigned long shapeOffset = handles[shape];
	if (shapeOffset == 0)
		return;
	ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(shapeOffset);
	unsigned numFrames = index->numFrames;
	for (unsigned i = 0; i < numFrames; i++)
	{
		index = (ShapeIndex *)CacheNodePtr::getPtr(shapeOffset);
		if (!(index->frames[i].offset & FRAME_ON_DISK))
			flushFrame(shapeOffset, i);
	}
	CacheNodePtr(shapeOffset - NODE_HEADER).free();
}

void ShapeHandler::flushFrame(unsigned long shapeOffset, unsigned short frame)
{
	ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(shapeOffset);
	if (index->numFrames <= frame)
		return;
	unsigned long frameOffset = index->frames[frame].offset;
	if (frameOffset == 0)
		return;
	if (frameOffset < 0x400)
		halt(__FILE__, 792);	// __LINE__
	frameOffset &= ~FRAME_FLAG;
	if (!(frameOffset & FRAME_ON_DISK))
	{
		CacheNodePtr node(frameOffset - NODE_HEADER);
		CacheNodePtr::deallocate(node);
	}
}

FrameHandler::FrameHandler(void) :
	CacheHandler((CacheNodeType)4)
{
	CacheNodePtr::attach(this, (CacheNodeType)4);
}

// A frame moved within the cache: repoint its frame table entry.
void FrameHandler::notifyMoved(unsigned long offset)
{
	FrameData *data = (FrameData *)CacheNodePtr::getPtr(offset);
	unsigned long shapeOffset = handles[data->shape];
	if (shapeOffset == 0)
		halt(__FILE__, 986);	// __LINE__
	ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(shapeOffset);
	unsigned long flag = index->frames[data->frame].offset & FRAME_FLAG;
	index->frames[data->frame].offset = offset | flag;
}

// A frame left the cache: put back the table entry it replaced.
void FrameHandler::cacheOut(unsigned long offset, unsigned)
{
	FrameData *data = (FrameData *)CacheNodePtr::getPtr(offset);
	unsigned long shapeOffset = handles[data->shape];
	if (shapeOffset == 0)
		halt(__FILE__, 999);	// __LINE__
	ShapeIndex *index = (ShapeIndex *)CacheNodePtr::getPtr(shapeOffset);
	index->frames[data->frame].offset = data->oldOffset;
}

void initShapeHandler(void)
{
	shapeHandler = new ShapeHandler;
	frameHandler = new FrameHandler;
	if (shapeHandler == 0 || frameHandler == 0)
		outOfMemory(__FILE__, 1016);	// __LINE__
}

void uninitShapeHandler(void)
{
	delete shapeHandler;
}
