// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: ARTLESS.C

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "..\UI\NEWGUMP.H"
#include "BASECAM.H"
#include "CGLOBVP.H"
#include "CRECT.H"
#include "CVPORT.H"
#include "DISPATCH.H"
#include "FONT.H"
#include "SYSTEM.H"

// Inline in the shared headers when this file was built.
inline SimpleVirtualString::operator char *(void) { return string; }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline void NewGump::commandKeyboard(Event &) {}

// A raised bevelled box: light on the top and left, dark on the bottom and right.
void Panel::drawup(Rect r, unsigned char color)
{
	DrawRectangle(GlobalVport::global_ptr, r.f_00, r.f_02, r.f_04, r.f_06, 0);
	DrawBox(GlobalVport::global_ptr, r.f_00 + 1, r.f_02 + 1, r.f_04 - 1, r.f_06 - 1, color);
	DrawLine(GlobalVport::global_ptr, r.f_00 + 1, r.f_02 + 1, r.f_00 + 1, r.f_06 - 2, color - 2);
	DrawLine(GlobalVport::global_ptr, r.f_00 + 2, r.f_02 + 1, r.f_04 - 2, r.f_02 + 1, color - 2);
	DrawLine(GlobalVport::global_ptr, r.f_00 + 2, r.f_06 - 1, r.f_04 - 1, r.f_06 - 1, color + 2);
	DrawLine(GlobalVport::global_ptr, r.f_04 - 1, r.f_02 + 2, r.f_04 - 1, r.f_06 - 2, color + 2);
}

void Panel::drawdown(Rect r, unsigned char color)
{
	DrawRectangle(GlobalVport::global_ptr, r.f_00, r.f_02, r.f_04, r.f_06, 0);
	DrawBox(GlobalVport::global_ptr, r.f_00 + 1, r.f_02 + 1, r.f_04 - 1, r.f_06 - 1, color);
	DrawLine(GlobalVport::global_ptr, r.f_00 + 1, r.f_02 + 1, r.f_00 + 1, r.f_06 - 1, color + 2);
	DrawLine(GlobalVport::global_ptr, r.f_00 + 2, r.f_02 + 1, r.f_04 - 1, r.f_02 + 1, color + 2);
}

PanelGump::PanelGump(NewGumpId id, NewGump *parent, Rect r, unsigned char c) :
	DrgNewGump(id, parent)
{
	f_1d |= 0x10;
	rect = r;
	color = c;
	font = GlobalFont::global_ptr;
}

void PanelGump::draw(short x, short y)
{
	Panel panel;
	Rect r = rect;
	r.moveto(x, y);
	panel.drawup(r, color);
}

void PanelGump::commandMouseLeft(Event &event)
{
	DrgNewGump::commandMouseLeft(event);
}

void PanelGump::printf(int x, int y, char *format, ...)
{
	va_list args;
	char buffer[256];
	if (font)
	{
		FORMAT_WORKSTRING(format, args);
		strcpy(buffer, WorkString);
		font->printf(get_dx() + x, get_dy() + y, rect.width() - x, buffer);
	}
}
