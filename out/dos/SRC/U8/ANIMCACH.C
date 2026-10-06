// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ANIMCACH.C

#include <alloc.h>
#include <mem.h>
#include "ERROR.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "ANIM.H"
#include "AHEADER.H"
#include "ANODE.H"
#include "ASTREAM.H"
#include "ANIMFILE.H"
#include "ANIMCACH.H"

// Inline in the shared headers when this file was built.
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }

char AnimCache::marker = 0;
AnimCacheHeader *AnimCache::animHeader = 0;
AnimFile AnimCache::file;
int AnimCache::typeArray[10];

void AnimCacheHeader::clear(void)
{
	memset(this, 0, sizeof(AnimCacheHeader));
}

void AnimCache::init(void)
{
	int i;

	for (i = 0; i < 10; i++)
		typeArray[i] = -1;
	animHeader = (AnimCacheHeader *)farmalloc(10 * sizeof(AnimCacheHeader));
	if (animHeader)
	{
		for (i = 0; i < 10; i++)
			animHeader[i].clear();
	}
	else
		halt(__FILE__, 54);	// __LINE__
}

void AnimCache::uninit(void)
{
	if (animHeader)
	{
		flush();
		farfree(animHeader);
		animHeader = 0;
	}
}

void AnimCache::free(short slot)
{
	int i;
	AnimStream *stream;

	for (i = 0; i < 64; i++)
	{
		if (animHeader[slot].streams[i])
		{
			stream = animHeader[slot].streams[i];
			stream->free();
			farfree(stream);
			animHeader[slot].setStream(i, 0);
		}
	}
	typeArray[slot] = -1;
}

int AnimCache::getSlot(unsigned short type)
{
	int i;

	for (i = 0; i < 10; i++)
		if (typeArray[i] == type)
			return i;
	return -1;
}

// Loads a shape's animations into the next slot, round robin over slots 1 to 9.
int AnimCache::load(unsigned short type)
{
	int slot;
	int i;
	AnimStream *stream;
	long offset;
	AnimHeader header;

	if (typeArray[0] == -1)
	{
		if (!file.is_valid())
			openFile();
		marker = 0;
		typeArray[0] = 1;
		load(1);
		if (type == 1)
			return 0;
	}
	free(marker);
	if (!file.getAnimHeader(type, &header))
		return -1;
	slot = marker;
	marker++;
	if (marker == 10)
		marker = 1;
	for (i = 0; i < 64; i++)
	{
		if (header.hasAnim((AnimSet)i))
		{
			stream = new AnimStream;
			if (!stream)
				halt(__FILE__, 161);	// __LINE__
			animHeader[slot].setStream(i, stream);
			offset = header.data[i];
			file.read((char *)&stream->header, 4, offset);
			stream->allocate();
			file.read((char *)stream->nodes, 8 * sizeof(AnimNode) * stream->header.frames);
		}
		else
			animHeader[slot].setStream(i, 0);
	}
	typeArray[slot] = type;
	return slot;
}

AnimCacheHeader *AnimCache::getHeader(unsigned short type)
{
	int slot = getSlot(type);
	if (slot == -1)
	{
		slot = load(type);
		if (slot == -1)
			return 0;
		return animHeader + slot;
	}
	return animHeader + slot;
}

void AnimCache::getNode(unsigned short type, AnimSet anim, char dir, short frame, AnimNode *node)
{
	AnimCacheHeader *header;
	AnimStream *stream;

	if (dir == 8)
		halt(__FILE__, 206);	// __LINE__
	header = getHeader(type);
	if (header->hasStream(anim))
	{
		stream = header->streams[anim];
		if (!stream)
			halt(__FILE__, 215);	// __LINE__
		*node = (stream->nodes + dir * stream->header.frames)[(unsigned char)frame];
	}
	else
		halt(__FILE__, 224);	// __LINE__
}

AnimStreamHeader AnimCache::getAnimStreamHeader(unsigned short type, AnimSet anim)
{
	AnimCacheHeader *header = getHeader(type);
	if (header && header->hasStream(anim))
	{
		AnimStream *stream = header->streams[anim];
		return stream->header;
	}
	AnimStreamHeader empty;
	return empty;
}

void AnimCache::flush(void)
{
	int i;

	if (file.is_valid())
		file.close();
	for (i = 0; i < 10; i++)
		free(i);
}

void AnimCache::openFile(void)
{
	file.open(fileSpec(0, StaticDir, animationFile, 0), ReadWrite);
}

void AnimCache::closeFile(void)
{
	file.close();
}

unsigned char AnimCache::isAnimInExistence(unsigned short type)
{
	if (getSlot(type) == -1)
	{
		AnimHeader header;
		if (!file.is_valid())
			openFile();
		if (!file.getAnimHeader(type, &header))
			return FALSE;
	}
	return TRUE;
}

unsigned char AnimCache::isAnimInExistence(unsigned short type, AnimSet anim)
{
	int slot = getSlot(type);
	if (slot == -1)
	{
		slot = load(type);
		if (slot == -1)
			return FALSE;
	}
	return animHeader[slot].hasStream(anim);
}
