// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: BASECAM.C

#include <conio.h>
#include <dos.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "..\UI\NEWGUMP.H"
#include "BASECAM.H"
#include "CDESC.H"
#include "CGLOBVP.H"
#include "CRECT.H"
#include "CTMPOINT.H"
#include "CVPORT.H"
#include "DISPATCH.H"
#include "FONT.H"
#include "SYSTEM.H"

// Inline in the shared headers when this file was built.
inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline Rect::Rect(void) {}
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline int Vport::getSeg(void) { return seg; }
inline GraphicYtable *Vport::getYTable(void) { return &graphicYtable; }
inline int GraphicYtable::getindex(short y) { return index[y]; }
inline void Vport::slam(Vport *from) { SlamScreen(getSeg(), MK_FP(from->getSeg(), from->getYTable()->getindex(0))); }

BaseCamera *theBaseCamera = 0;
unsigned char BaseCamera::needSlam = 0;
unsigned char BaseCamera::inTextMode = 0;
unsigned char BaseCamera::okToShow = 1;
unsigned char BaseCamera::alternateFrame = 0;
Rect BaseCamera::graphics;
Rect BaseCamera::text;
Rect BaseCamera::screen;

void BaseCamera::textVideoMode(void)
{
	if (!inTextMode)
	{
		inTextMode = 1;
		theMouse->suspend();
		asm {
			mov al, 83h
			mov ah, 0
			int 10h
			mov ax, 1112h
			xor bl, bl
			int 10h
			mov al, 0Ah
			mov bx, 0
			mov cx, 0FFFFh
			mov dx, 5749h
			int 33h
			mov ax, 1
			int 33h
			mov ax, 1Eh
		}
		screen = text;
	}
}

void BaseCamera::initPalette(void)
{
}

void BaseCamera::graphicVideoMode(void)
{
	if (inTextMode)
	{
		inTextMode = 0;
		asm {
			mov al, 93h
			mov ah, 0
			int 10h
		}
		theMouse->resume();
		window(1, 1, 80, 25);
		gotoxy(1, 1);
		initPalette();
		screen = graphics;
	}
}

void BaseCamera::flipVideoMode(void)
{
	if (inTextMode)
		graphicVideoMode();
	else
		textVideoMode();
}

void BaseCamera::show(short x1, short y1, short x2, short y2, unsigned char)
{
	dispatcher->refresh(Rect(x1, y1, x2, y2));
}

void BaseCamera::show(void)
{
	if (okToShow)
	{
		GlobalVport::global_ptr->clear(0);
		show(screen.f_00, screen.f_02, screen.f_04, screen.f_06, 0);
		needSlam = 1;
		slam();
	}
}

void BaseCamera::slam(void)
{
	if (!needSlam)
		return;
	TMMousePointer::before_slam(GlobalVport::global_ptr);
	GlobalVport::main_screen->slam(GlobalVport::global_ptr);
	TMMousePointer::after_slam(GlobalVport::global_ptr);
	needSlam = 0;
}
