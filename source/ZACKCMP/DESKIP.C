// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r- -y
// name: ..\ZACKCMP\DESKIP.C

#include <alloc.h>
#include <dos.h>
#include <io.h>
#include <mem.h>
#include <stdio.h>
#include "CFILE.H"
#include "CVPORT.H"
#include "CGLOBVP.H"
#include "ERROR.H"
#include "FEXIST.H"
#include "AMUSIC.H"
#include "CEXIT.H"
#include "WRITSKP.H"
#include "FVINFO.H"
#include "DESKIP.H"

// Inline in the shared headers when this file was built.
inline Index::Index(void) { size = 0; offset = 0; }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline SharedFile::SharedFile(void) {}
inline FlexFile::FlexFile(void) {}
inline int Vport::getSeg(void) { return seg; }
inline int GraphicYtable::getindex(short y) { return index[y]; }
inline GraphicYtable *Vport::getYTable(void) { return &graphicYtable; }
inline Point::Point(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { f_00 = x1; f_02 = y1; f_04 = x2; f_06 = y2; }
inline Rect::Rect(void) { set(0, 0, 319, 199); }
inline GraphicYtable::GraphicYtable(void) { f_04 = 2; index = 0; }
inline Vport::Vport(void) { f_0f = 2; }
inline Vport::~Vport(void) { free(); }
inline void Vport::slam(Vport *v)
{
	SlamScreen(getSeg(), MK_FP(v->getSeg(), v->getYTable()->getindex(0)));
}

#define max(a, b) (((a) > (b)) ? (a) : (b))

inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }

// Linear address of the selector behind a pointer.
inline unsigned long selectorBase(void *p)
{
	return LocalDescriptorTable::table[FP_SEG(p) >> 3].getBase();
}

unsigned invisible = 0;
long totalBytes = 0;
unsigned typeNumber;
char escapeBytes[5];
unsigned char isEscapeByte[256];
unsigned char *outputFrameBuffer;

void slam(Vport *v)
{
	GlobalVport::main_screen->slam(v);
}

void waitKey(void)
{
}

void checkKey(void)
{
}

// Draws a frame's skip table at x, y into vp, which is cleared to 0xff
// first. A run byte that is an escape byte copies that many pixels from
// the same place in ref instead.
void reconstruct_skipdraw(Vport *vp, short x, short y, unsigned char *data, Vport *ref)
{
	int col;
	int row;
	int left;
	int top;
	int right;
	int bottom;
	int xoff;
	int count;
	int color;
	int flags;
	unsigned char *src;
	short *header;
	unsigned refSeg;
	unsigned destSeg;
	unsigned refOff;
	unsigned destOff;

	src = data + sizeof(SkipHeader);
	header = (short *)data;
	flags = *header++;
	xoff = header[2];
	left = x - xoff;
	top = y - header[3];
	right = left + header[0] - 1;
	bottom = top + header[1] - 1;
	vp->fill(left, top, right, bottom, 0xff);
	refSeg = 0;
	refOff = 0;
	if (ref)
		refOff = FP_OFF(MK_FP(ref->seg, ref->graphicYtable.index[0]));
	destOff = FP_OFF(MK_FP(vp->seg, vp->graphicYtable.index[0]));
	if (ref)
		refSeg = ref->seg;
	destSeg = vp->seg;
	if (flags & SKIP_RLE)
		for (row = top; row <= bottom; row++)
		{
			unsigned destRow;
			unsigned refRow;
			unsigned char *dest;
			unsigned char *refp;
			char n;

			destRow = row * 320 + destOff;
			refRow = row * 320 + refOff;
			dest = (unsigned char *)MK_FP(destSeg, destRow);
			refp = (unsigned char *)MK_FP(refSeg, refRow);
			col = left;
			while (col <= right)
			{
				count = *src++;
				col += count;
				if (col > right)
					continue;
				count = *src++;
				if (count & 1)
				{
					count >>= 1;
					color = *src++;
					*(unsigned *)&dest = destRow + col;
					col += count;
					while (count--)
						*dest++ = color;
					continue;
				}
				count >>= 1;
				while (count--)
				{
					*(unsigned *)&dest = destRow + col;
					n = isEscapeByte[*src];
					if (n)
					{
						if (n == -1)
						{
							n = *++src;
							count--;
						}
						*(unsigned *)&refp = refRow + col;
						col += n;
						while (n--)
							*dest++ = *refp++;
					}
					else
					{
						*dest++ = *src;
						col++;
					}
					src++;
				}
			}
		}
	else
		for (row = top; row <= bottom; row++)
		{
			unsigned destRow;
			unsigned refRow;
			unsigned char *dest;
			unsigned char *refp;
			char n;

			destRow = row * 320 + destOff;
			refRow = row * 320 + refOff;
			dest = (unsigned char *)MK_FP(destSeg, destRow);
			refp = (unsigned char *)MK_FP(refSeg, refRow);
			col = left;
			while (col <= right)
			{
				count = *src++;
				col += count;
				if (col > right)
					continue;
				count = *src++;
				while (count--)
				{
					*(unsigned *)&dest = destRow + col;
					n = isEscapeByte[*src];
					if (n)
					{
						if (n == -1)
						{
							n = *++src;
							count--;
						}
						*(unsigned *)&refp = refRow + col;
						col += n;
						while (n--)
							*dest++ = *refp++;
					}
					else
					{
						*dest++ = *src;
						col++;
					}
					src++;
				}
			}
		}
}

unsigned char *SkipShape::getFrame(int frame)
{
	unsigned long base = selectorBase(this) + FP_OFF(this);
	long offset = frames[frame].offset & 0x7fffffffL;
	unsigned long address = base + offset;
	unsigned char *p = (unsigned char *)makePtr(address);
	return p;
}

ZackFile::~ZackFile(void) { if (buffer) farfree(buffer); }

ZackFile::ZackFile(void)
{
	used = 0;
	buffering = 0;
	buffer = 0;
	bufferSize = 500000L;
	while (buffer == 0 && bufferSize > 65000L)
	{
		buffer = (char *)farmalloc(bufferSize);
		bufferSize -= 10000;
	}
	bufferBase = 0;
	if (buffer)
		bufferBase = selectorBase(buffer);
	else
		buffering = 0;
}

void ZackFile::startBuffering(void)
{
	buffering = 1;
	if (buffer == 0)
		buffering = 0;
}

void ZackFile::stopBuffering(void)
{
	if (buffering)
		dumpIt();
	buffering = 0;
}

void ZackFile::dumpIt(void)
{
	if (buffering)
	{
		BaseFile::write(buffer, used);
		used = 0;
	}
}

void ZackFile::seek(long pos, SeekMode mode)
{
	if (buffering)
		halt(__FILE__, 360);	// __LINE__
	else
		BaseFile::seek(pos, mode);
}

long ZackFile::tell(void)
{
	return BaseFile::tell() + (buffering ? used : 0);
}

long ZackFile::read(char *data, long count, long pos)
{
	if (buffering)
		halt(__FILE__, 373);	// __LINE__
	else
		return BaseFile::read(data, count, pos);
}

long ZackFile::write(char *data, long count, long pos)
{
	if (buffering)
	{
		if (bufferSize - used < count)
			dumpIt();
		memcpy(makePtr(bufferBase + used), data, count);
		used += count;
	}
	else
		return BaseFile::write(data, count, pos);
}

void ZackFile::create(char *name, short count)
{
	FlexFile::create(name, count);
}

// Rebuilds a compressed shape record as skip tables written to file. Each
// frame is drawn over the previous one, so its escape runs can copy from it.
void decompressShape(char *data, ZackFile &file, Vport &front, Vport &back, Vport &work)
{
	Vport *cur;
	Vport *prev;
	int i;
	SkipShape *shape;
	int frameCount;
	int maxWidth;
	int maxHeight;
	int maxXoff;
	int maxYoff;
	int frame;
	int originX;
	int originY;
	long start;
	SkipHeader *first;

	front.clear(0xff);
	back.clear(0xff);
	work.clear(0xff);
	cur = &front;
	prev = &back;
	memcpy(escapeBytes, data, 5);
	memset(isEscapeByte, 0, 256);
	for (i = 0; i < 5; i++)
		isEscapeByte[escapeBytes[i]] = i + 2;
	isEscapeByte[0xff] = 0xff;
	shape = (SkipShape *)(data + 5);
	frameCount = shape->frameCount;
	maxWidth = 0;
	maxHeight = 0;
	maxXoff = 0;
	maxYoff = 0;
	for (frame = 0; frame < frameCount; frame++)
	{
		SkipHeader *h = (SkipHeader *)shape->getFrame(frame);

		maxWidth = max(maxWidth, max(h->width, h->xoff));
		maxXoff = max(maxXoff, h->xoff);
		maxHeight = max(maxHeight, max(h->height, h->yoff));
		maxYoff = max(maxYoff, h->yoff);
	}
	originX = maxWidth > 150 ? maxXoff + 7 : 150;
	originY = maxHeight > 150 ? maxYoff + 7 : 150;
	start = file.tell();
	first = (SkipHeader *)shape->getFrame(0);
	reconstruct_skipdraw(cur, originX, originY, (unsigned char *)first, 0);
	Rect rect(originX - first->xoff,
		originY - first->yoff,
		originX - first->xoff + first->width - 1,
		originY - first->yoff + first->height - 1);
	int size = WriteSkip(*cur, rect.f_00, rect.f_02, rect.f_04, rect.f_06,
		originX, originY, outputFrameBuffer, 0xff, 1);
	file.write((char *)shape, shape->frameCount * sizeof(FrameIndex) + 6, start);
	file.startBuffering();
	shape->frames[0].offset = (file.tell() - start) | 0x80000000L;
	{
		FrameOwner header;

		header.shape = typeNumber;
		header.frame = 0;
		header.unknown = 0;
		file.write((char *)&header, sizeof(header));
	}
	file.write((char *)outputFrameBuffer, (unsigned)size);
	shape->frames[0].length = size + 8;
	frameCount = shape->frameCount;
	for (frame = 1; frame < frameCount; frame++)
	{
		if ((frame & 31) == 0)
		{
			long percent = file.tell() * 100 / totalBytes;
			trace(TRACE_ALWAYS, "\r%03ld%%", percent);
		}
		prev->clear(0xff);
		SkipHeader *h = (SkipHeader *)shape->getFrame(frame);
		char xshift = 0;
		char yshift = 0;
		reconstruct_skipdraw(prev, originX + xshift, originY + yshift, (unsigned char *)h, cur);
		Rect rect(originX - h->xoff + xshift,
			originY - h->yoff + yshift,
			originX - h->xoff + xshift + h->width - 1,
			originY - h->yoff + yshift + h->height - 1);
		int size = WriteSkip(*prev, rect.f_00, rect.f_02, rect.f_04, rect.f_06,
			originX, originY, outputFrameBuffer, 0xff, 1);
		if (invisible == 0)
		{
			GlobalVport::main_screen->clear(0xff);
			SkipDraw(GlobalVport::main_screen, originX, originY, outputFrameBuffer);
		}
		shape->frames[frame].offset = (file.tell() - start) | 0x80000000L;
		FrameOwner header;
		header.shape = typeNumber;
		header.frame = frame;
		header.unknown = 0;
		file.write((char *)&header, sizeof(header));
		file.write((char *)outputFrameBuffer, (unsigned)size);
		h = (SkipHeader *)shape->getFrame(frame);
		shape->frames[frame].length = size + 8;
		Vport *swap = cur;
		cur = prev;
		prev = swap;
		DrawBox(prev, 0, 0, 319, 199, 0xff);
		checkKey();
	}
	file.stopBuffering();
	file.write((char *)shape, shape->frameCount * sizeof(FrameIndex) + 6, start);
}

// Rebuilds every shape in a flex file as skip tables, into dest.
void deskip(char *source, char *dest, char *)
{
	Index *indexes;
	Index *newIndexes;
	char *shape;
	int i;

	invisible = 1;
	if (FileExists(source) && FileExists(dest))
	{
		trace(TRACE_ALWAYS,
			"--------------------------------------------------------------------\r\n"
			"An incomplete installation has been detected. Please re-install.\r\n"
			"Une installation partielle a \202t\202 d\202c\202l\202e. Veuillez reinstaller.\r\n"
			"Unvollst\204ndige Installation entdeckt. Bitte installieren Sie erneut.\r\n");
		musicSlowStop();
		Exit(0);
	}
	FlexFile in(source, ReadWrite, -1);
	if (!in.is_valid())
		halt(__FILE__, 658);	// __LINE__
	indexes = new Index[2048];
	newIndexes = new Index[2048];
	if (indexes == 0 || newIndexes == 0)
		outOfMemory(__FILE__, 671);	// __LINE__
	in.read((char *)indexes, 2048L * sizeof(Index), 0x80);
	memset(newIndexes, 0, 2048 * sizeof(Index));
	outputFrameBuffer = (unsigned char *)farmalloc(0xfffe);
	if (outputFrameBuffer == 0)
		outOfMemory(__FILE__, 678);	// __LINE__
	FlexHeader header;
	in.getHeader(&header);
	totalBytes = header.fileSize;
	shape = (char *)farmalloc(indexes[1].size);
	if (shape == 0)
		outOfMemory(__FILE__, 692);	// __LINE__
	Vport front; front.alloc(0);
	Vport back; back.alloc(0);
	Vport work; work.alloc(0);
	ZackFile out;
	out.create(dest, 2048);
	for (i = 0; i < 2048; i++)
	{
		typeNumber = i;
		checkKey();
		if (indexes[i].size)
		{
			long start;
			long end;

			in.readRecord(i, shape, indexes[i].size);
			out.seek(out.size());
			start = out.tell();
			decompressShape(shape, out, front, back, work);
			out.seek(out.size());
			end = out.tell();
			newIndexes[i].size = end - start;
			newIndexes[i].offset = start;
			in.seek(indexes[i].offset);
			in.setSize(tell(in.handle));
		}
	}
	farfree(shape);
	farfree(outputFrameBuffer);
	FlexHeader newHeader;
	in.getHeader(&newHeader);
	out.setHeader(&newHeader);
	out.write((char *)newIndexes, 2048L * sizeof(Index), 0x80);
	unlink(source);
	delete indexes;
	delete newIndexes;
}
