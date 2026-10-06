#ifndef CVPORT_H
#define CVPORT_H

#include "CDESC.H"
#include "CRECT.H"
#include "CVIDMODE.H"
#include "CEXIT.H"

class Vport
{
public:
	unsigned seg;
	GraphicYtable graphicYtable;
	Rect rect;
	unsigned char f_0f;
	unsigned char f_10;

	Vport(void);
	~Vport(void);
	unsigned char alloc(int);
	void map_to_screen(unsigned char);
	unsigned char init_ytable(unsigned short);
	void free(void);
	Vport &operator=(Vport &);
	void copy_to(Vport *);
	void copy_from(Vport *);
	void clear(unsigned char);
	void fill(Rect &, unsigned char);
	void fill(short, short, short, short, unsigned char);
	void copy_to(Vport *, short, short, Rect);
	void slam(Vport *);
	int getSeg(void);
	GraphicYtable *getYTable(void);
	Rect *getRect(void);
};

// Puts back the video mode that was current when it was made.
class VideoRestore : public VideoMode, public ExitSystem
{
public:
	VideoRestore(void) { grab(); }
	virtual void exit_code(void);
};

extern "C" void CopyViewport(Vport *, Vport *);
extern "C" void ClearViewport(Vport *, unsigned char);
extern "C" void DrawBox(Vport *, short, short, short, short, unsigned char);
extern "C" void DrawRectangle(Vport *, short, short, short, short, unsigned char);
extern "C" void DrawLine(Vport *, short, short, short, short, unsigned char);
extern "C" void SlamScreen(unsigned, void *);

extern unsigned VgaScreenDescriptor;
extern unsigned char SegmentInterval[2];

#endif
