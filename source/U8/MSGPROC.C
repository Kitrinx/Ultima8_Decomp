// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MSGPROC.C

#include "CVPORT.H"
#include "CCURRVP.H"
#include "CGLOBVP.H"
#include "MSGPROC.H"
#include "PAGAN.H"

inline Point::Point(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline void Rect::set(short x1, short y1, short x2, short y2) { Point::set(x1, y1); f_04 = x2; f_06 = y2; }
inline Rect::Rect(void) {}
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline GraphicYtable::GraphicYtable(void) { f_04 = 2; index = 0; }
inline Vport::Vport(void) { seg = 0; f_0f = 2; f_10 = 0; }
inline Vport::~Vport(void) { free(); }
inline CacheFont::~CacheFont(void) {}

unsigned Msg::fonttype;
unsigned char Msg::credits;

// Splits the text into lines at '*' and where it gets too wide; '~' narrows it.
Msg::Msg(PaganProcess *owner, char *message)
{
	if (fonttype == 0)
		font.setFont((FontType)14);
	else
		font.setFont((FontType)6);
	if (fonttype == 0)
		titleFont.setFont((FontType)15);
	else
		titleFont.setFont((FontType)8);
	process = owner;
	offScreen = 0;
	text = message;
	numLines = 1;
	int i = 0;
	int line = 0;
	lines[line++] = text;
	int width = 0;
	int maxWidth = 260;
	highlight = 0;
	indent = 0;
	for (; text[i] != 0; i++)
	{
		if (text[i] != '*')
		{
			if (text[i] == '~')
				maxWidth = 200;
			else
			{
				width += font.width(text[i]);
				if (width >= maxWidth)
				{
					while (text[i--] != ' ')
						;
					i++;
					width = 0;
					text[i] = 0;
					numLines++;
					lines[line++] = &text[i + 1];
				}
			}
		}
		else
		{
			width = 0;
			text[i] = 0;
			numLines++;
			lines[line++] = &text[i + 1];
		}
	}
	y = 210;
	allShown = 0;
	scroll = 1;
}

// Draws each line by its marker: '+' a title, '&' and '}' the title font.
void Msg::run(void)
{
	indent = 0;
	highlight = 0;
	if (offScreen == 0)
	{
	for (int i = 0; i < numLines; i++)
	{
		if (*lines[i] == '+')
		{
			Rect rect(0, 0, 319, 50);
			Vport vport;
			vport = *GlobalVport::global_ptr;
			vport.rect = rect;
			CurrentVport current(&vport);
			font.printf(0, 35, 'c', lines[i] + 1);
			font.printf(0, 45, 'c', "&&&&&&&&&&&&&&&&&&&&&&&&");
			allShown = 1;
			offScreen = 1;
		}
		else if (*lines[i] == '&' && credits)
		{
			titleFont.printf(40, y + i * font.height('A'), 'c', lines[i] + 1);
			highlight = 1;
		}
		else if (*lines[i] == '&')
		{
			font.printf(40, y + i * font.height('A'), 'c', lines[i] + 1);
			highlight = 1;
		}
		else if (*lines[i] == '}')
		{
			font.printf(40, y + i * font.height('A'), 'c', lines[i] + 1);
			highlight = 1;
			indent = 1;
		}
		else if (indent && highlight)
			font.printf(80, y + i * titleFont.height('A'), 'c', lines[i]);
		else if (highlight)
			titleFont.printf(40, y + i * font.height('A'), 'c', lines[i]);
		else if (indent)
			titleFont.printf(80, y + i * titleFont.height('A'), lines[i]);
		else if (*lines[i] == '~')
		{
			titleFont.printf(80, y + i * titleFont.height('A'), lines[i] + 1);
			indent = 1;
		}
		else
			font.printf(40, y + i * font.height('A'), lines[i]);
	}
	if (credits)
	{
		if (y + numLines * font.height('A') + 10 < 200)
			allShown = 1;
		if (y + numLines * font.height('A') + 10 < 0)
			offScreen = 1;
	}
	else
	{
		if (y + numLines * font.height('A') + 70 < 200)
			allShown = 1;
		if (y + numLines * font.height('A') + 70 < 0)
			offScreen = 1;
	}
	if (scroll)
		y--;
	}
	else
		process->kill(this);
}
