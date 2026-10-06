// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: FORMAT.C

#include "CEXIT.H"
#include "CRECT.H"
#include "CAFONT.H"
#include "SYSTEM.H"
#include "FORMAT.H"

inline SimpleVirtualString::SimpleVirtualString(short length) { string = 0; alloc(length); }
inline SimpleVirtualString::~SimpleVirtualString(void) { free(); }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline CacheFont::~CacheFont(void) {}

// Text markup: '~' breaks the line, '*' breaks the page, '%' indents.
// Lines end in '\n', pages in '\xff'.
enum FormatState
{
	FORMAT_DONE,
	FORMAT_COPY,
	FORMAT_AFTER_SPACE,
	FORMAT_AFTER_STOP,
	FORMAT_NEW_LINE,
	FORMAT_NEW_PAGE,
	FORMAT_NEXT_RECT,
	FORMAT_START_LINE,
	FORMAT_BREAK_LINE,
	FORMAT_BREAK_PAGE,
	FORMAT_QUOTE,
	FORMAT_END
};

// Lays text out over a cycle of page rectangles; returns the laid-out text.
char *printFormat(char *text, int fontType, Rect *rects, int numRects)
{
	static SimpleVirtualString buffer(10240);
	CacheFont font((FontType)fontType);
	char *src;
	char *dst;
	char *spaceSrc;
	char *spaceDst;
	char *stopSrc;
	char *stopDst;
	char *quote;
	char *end;
	unsigned width;
	unsigned height;
	unsigned x;
	unsigned y;
	int charWidth;
	int charHeight;
	int rect;
	int state;

	src = text;
	dst = buffer.string;
	end = buffer.string + buffer.size;
	rect = numRects - 1;
	state = FORMAT_NEXT_RECT;
	while (state != FORMAT_DONE)
	{
		if (dst >= end)
			halt(__FILE__, 161);	// __LINE__
		switch (state)
		{
		case FORMAT_COPY:
			*dst++ = *src++;
			font.dim(*src, charWidth, charHeight);
			x += charWidth;
			switch (src[-1])
			{
			case 0:
				dst--;
				state = FORMAT_END;
				break;
			case '%':
				dst[-1] = ' ';
				*dst++ = ' ';
				*dst++ = ' ';
				*dst++ = ' ';
				x += charWidth * 3;
			case ' ':
				state = FORMAT_AFTER_SPACE;
				break;
			case ',':
			case '!':
			case ';':
			case '.':
			case ':':
			case '?':
				state = FORMAT_AFTER_STOP;
				break;
			case '"':
				state = FORMAT_QUOTE;
				break;
			case '~':
				dst--;
				state = FORMAT_BREAK_LINE;
				break;
			case '*':
				dst--;
				state = FORMAT_BREAK_PAGE;
				break;
			default:
				if (x >= width)
					state = FORMAT_NEW_LINE;
				else
					state = FORMAT_COPY;
				break;
			}
			break;
		case FORMAT_AFTER_SPACE:
			spaceSrc = src;
			spaceDst = dst;
			state = FORMAT_COPY;
			break;
		case FORMAT_AFTER_STOP:
			stopSrc = src;
			stopDst = dst;
			state = FORMAT_COPY;
			break;
		case FORMAT_QUOTE:
			if (quote)
			{
				quote = 0;
				state = FORMAT_AFTER_STOP;
			}
			else
			{
				quote = src;
				state = FORMAT_COPY;
			}
			break;
		case FORMAT_BREAK_LINE:
			spaceSrc = spaceDst = 0;
			state = FORMAT_NEW_LINE;
			break;
		case FORMAT_NEW_LINE:
			if (spaceSrc)
			{
				src = spaceSrc;
				dst = spaceDst;
			}
			*dst++ = '\n';
			y += charHeight;
			if (y >= height)
				state = FORMAT_NEW_PAGE;
			else
				state = FORMAT_START_LINE;
			break;
		case FORMAT_START_LINE:
			x = 0;
			spaceSrc = spaceDst = 0;
			stopSrc = stopDst = 0;
			quote = 0;
			while (*src == ' ' || *src == '\t')
				src++;
			state = FORMAT_COPY;
			break;
		case FORMAT_BREAK_PAGE:
			stopSrc = stopDst = spaceSrc = spaceDst = 0;
			state = FORMAT_NEW_PAGE;
			break;
		case FORMAT_NEW_PAGE:
			if (stopSrc)
			{
				src = stopSrc;
				dst = stopDst;
			}
			else if (spaceSrc)
			{
				src = spaceSrc;
				dst = spaceDst;
			}
			while (dst[-1] == '\xff')
				dst--;
			*dst++ = '\xff';
			state = FORMAT_NEXT_RECT;
			break;
		case FORMAT_NEXT_RECT:
			rect++;
			if (rect == numRects)
				rect = 0;
			width = rects[rect].width();
			height = rects[rect].f_06 - rects[rect].f_02;
			font.dim(*src, charWidth, charHeight);
			height -= height % charHeight;
			x = 0;
			y = 0;
			state = FORMAT_START_LINE;
			break;
		case FORMAT_END:
			while (dst[-1] == '\xff' || dst[-1] == ' ' || dst[-1] == '\n')
				dst--;
			*dst = 0;
			// A short last page: join it to the one before.
			if (y == 0 && width / 2 >= x)
			{
				for (char *p = dst; p != buffer.string; p--)
				{
					if (*p == '\n')
					{
						*p = '\xff';
						break;
					}
					if (*p == '\xff')
						*p = '\n';
				}
			}
			state = FORMAT_DONE;
			break;
		default:
			halt(__FILE__, 367);	// __LINE__
		}
	}
	return buffer.string;
}
