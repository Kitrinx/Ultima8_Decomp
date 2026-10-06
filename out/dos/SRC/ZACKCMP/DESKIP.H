#ifndef DESKIP_H
#define DESKIP_H

#include "..\POLIB\INC\FLEX.H"
#include "CFILE.H"

class Vport;

// One frame's place in a shape record; the top bit of offset marks a
// frame already rebuilt by decompressShape.
struct FrameIndex
{
	long offset;
	short length;
};

// A shape record: a header, then one FrameIndex per frame.
class SkipShape
{
public:
	short maxWidth;
	short maxHeight;
	short frameCount;
	FrameIndex frames[1];

	unsigned char *getFrame(int);
};

// A frame's skip table header; the row data follows.
struct SkipHeader
{
	short flags;
	unsigned short width;
	unsigned short height;
	short xoff;
	short yoff;
};

// What precedes each rebuilt frame's skip table.
struct FrameOwner
{
	short shape;
	short frame;
	long unknown;
};

// A flex file whose writes can be gathered in a large memory buffer.
class ZackFile : public FlexFile
{
public:
	char *buffer;
	long bufferSize;
	long used;
	int buffering;
	unsigned long bufferBase;	// linear address of buffer

	ZackFile(void);
	virtual long read(char *, long, long = -1);
	virtual long write(char *, long, long = -1);
	virtual ~ZackFile(void);
	void startBuffering(void);
	void stopBuffering(void);
	void dumpIt(void);
	void seek(long, SeekMode = FromStart);
	long tell(void);
	void create(char *, short);
};

void slam(Vport *);
void waitKey(void);
void checkKey(void);
void reconstruct_skipdraw(Vport *, short, short, unsigned char *, Vport *);
void decompressShape(char *, ZackFile &, Vport &, Vport &, Vport &);
void deskip(char *, char *, char *);

extern unsigned invisible;
extern long totalBytes;
extern unsigned typeNumber;
extern char escapeBytes[5];
extern unsigned char isEscapeByte[256];
extern unsigned char *outputFrameBuffer;

#endif
