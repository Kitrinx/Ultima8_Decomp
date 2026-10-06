// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ANIMFILE.C

#include <io.h>
#include <string.h>
#include "ERROR.H"
#include "FILESPEC.H"
#include "AHEADER.H"
#include "ANODE.H"
#include "ASTREAM.H"
#include "ANIMFILE.H"

// Inline in the shared headers when this file was built.
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline FileSpec::FileSpec(char *path, int attrib) { become(path, attrib); }

char animationFile[] = "anim.dat";

AnimFile::AnimFile(char *name) : BaseFile(name, RWShareReadWrite)
{
	if (strlen(name) > 80)
		halt(__FILE__, 27);	// __LINE__
	strcpy(animName, name);
}

// Writes an empty table of shape offsets.
void AnimFile::initialize(char *name)
{
	int i;
	long zero = 0;

	create(name, ReadWrite);
	setFilename(name);
	for (i = 0; i < 0x800; i++)
		write((char *)&zero, 4);
	close();
}

Boolean AnimFile::load(unsigned short shape, AnimSet anim, AnimStream *stream)
{
	AnimHeader header;

	if (!getAnimHeader(shape, &header))
		return FALSE;
	if (!header.hasAnim(anim))
		return FALSE;
	read((char *)&stream->header, 4, header.data[anim]);
	stream->allocate();
	read((char *)stream->nodes, 8 * sizeof(AnimNode) * stream->header.getFrames());
	return TRUE;
}

// Appends the animation to the file; a null stream removes it.
void AnimFile::save(unsigned short shape, AnimSet anim, AnimStream *stream)
{
	long offset;
	AnimHeader header;

	if (!getAnimHeader(shape, &header))
		createHeader(shape);
	if (stream)
	{
		if (!stream->nodes)
			halt(__FILE__, 83);	// __LINE__
		offset = size();
		header.setAnim(anim, offset);
		setAnimHeader(shape, &header);
		write((char *)&stream->header, 4, offset);
		write((char *)stream->nodes, 8 * sizeof(AnimNode) * stream->header.getFrames());
	}
	else
	{
		header.clearAnim(anim);
		setAnimHeader(shape, &header);
	}
}

// Rewrites the file without its dead space and returns the new size.
long AnimFile::optimize(void)
{
	int shape;
	int anim;
	AnimFile temp;
	AnimHeader header;
	AnimStream stream;
	long offset;
	long newSize;
	FileSpec spec(animName, 0);

	strcpy(spec.name, "anim_opt");
	open(animName, RWShareReadWrite);
	temp.initialize(spec);
	temp.open(temp.filename, ReadWrite);
	for (shape = 0; shape < 0x800; shape++)
	{
		if (getAnimHeader(shape, &header) && header.isData())
		{
			temp.setAnimHeader(shape, &header);
			for (anim = 0; anim < 64; anim++)
			{
				if (header.hasAnim((AnimSet)anim))
				{
					offset = header.data[anim];
					read((char *)&stream.header, 4, offset);
					if (stream.header.getFrames())
					{
						stream.allocate();
						read((char *)stream.nodes, 8 * sizeof(AnimNode) * stream.header.frames);
						temp.save(shape, (AnimSet)anim, &stream);
					}
					else
						temp.save(shape, (AnimSet)anim, 0);
				}
			}
		}
	}
	newSize = temp.size();
	relocate(0, 0, temp.size(), &temp);
	setSize(::tell(handle));
	temp.close();
	temp.remove();
	close();
	return newSize;
}

Boolean AnimFile::getAnimHeader(unsigned short shape, AnimHeader *header)
{
	long offset;

	read((char *)&offset, 4, shape * 4);
	if (offset)
	{
		read((char *)header, sizeof(AnimHeader), offset);
		return TRUE;
	}
	return FALSE;
}

Boolean AnimFile::setAnimHeader(unsigned short shape, AnimHeader *header)
{
	long offset;

	read((char *)&offset, 4, shape * 4);
	if (offset)
	{
		write((char *)header, sizeof(AnimHeader), offset);
		return TRUE;
	}
	return FALSE;
}

// Appends an empty header for the shape and points its table entry at it.
void AnimFile::createHeader(unsigned short shape)
{
	long offset = size();
	long pos;
	AnimHeader header;

	pos = shape * 4;
	write((char *)&offset, 4, pos);
	write((char *)&header, sizeof(AnimHeader), offset);
}

void AnimFile::clearHeader(unsigned short shape)
{
	long zero = 0;
	long pos = shape * 4;

	write((char *)&zero, 4, pos);
}

void AnimFile::setFilename(char *name)
{
	if (strlen(name) > 80)
		halt(__FILE__, 283);	// __LINE__
	strcpy(animName, name);
	BaseFile::setFilename(animName);
}
