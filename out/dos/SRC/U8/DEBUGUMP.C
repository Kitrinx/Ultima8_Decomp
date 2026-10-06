// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: DEBUGUMP.C

#include <alloc.h>
#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include "..\UI\NEWGUMP.H"
#include "BASECAM.H"
#include "CDESC.H"
#include "CRECT.H"
#include "CTRACKER.H"
#include "DEBUGGER.H"
#include "DISPATCH.H"
#include "ERROR.H"
#include "FILESPEC.H"
#include "FLAG.H"
#include "INTER.H"
#include "SYSTEM.H"
#include "TEXTGUMP.H"
#include "WORLD.H"
#include "YAMM.H"
#include "DEBUGUMP.H"

// Menu and hot key commands.
#define CMD_LOAD_UNIT		0
#define CMD_VIEW_FILE		1
#define CMD_RUN			2
#define CMD_RUN_TO_CURSOR	3
#define CMD_TRACE_INTO		4
#define CMD_STEP_OVER		5
#define CMD_RUN_TO_RETURN	6
#define CMD_BREAKPOINT		7
#define CMD_SEARCH_AGAIN	9
#define CMD_GOTO_LINE		10
#define CMD_WATCH		11
#define CMD_INSPECT		12
#define CMD_CLEAR_WATCH		13
#define CMD_CHANGE_FLAG		14
#define CMD_FIND		15
#define CMD_BREAK		16

#define NUM_WATCHES		10

inline void WindowGump::setColor(char c) { color = c; }
inline void *Yamm::resolve(unsigned short offset) { return (char *)buffer + offset; }
inline YammList::YammList(unsigned short id, unsigned short elementSize, int strings)
{
	list = id;
	size = elementSize;
	isString = strings;
}

extern "C" char UsecodeDir[];

unsigned char DebugGump::init = 0;

// Symbol type codes used in a routine's symbol table.
static struct
{
	char type;
	char *name;
} symbolTypes[9] = {
	{0, "STT_ERROR"},
	{'$', "STT_STRUCTURE"},
	{'b', "STT_BYTE"},
	{'i', "STT_INTEGER"},
	{'d', "STT_LONG"},
	{'p', "STT_POINTER"},
	{'s', "STT_STRING"},
	{'%', "STT_NAMED_TYPE"},
	{'l', "STT_LIST"}
};

char *DebugGump::watches[NUM_WATCHES];
unsigned char DebugGump::fromInterpreter;
ViewFile *DebugScroller::viewFile;

void initDebugger(void)
{
}

void SourceDebugger::atBreakPoint(void)
{
	DebugGump *gump = new DebugGump(NewGumpId(0), 0, Rect(0, 0, 79, 49), this, 1);
	if (stricmp(debugger->currentFile(), gump->scroller->getSourceFile()))
	{
		gump->scroller->setFile(debugger->currentFile());
		gump->scroller->cursorY = debugger->currentLine - 1;
	}
	((World *)dispatcher->getBase())->flags = 0;
	Dispatch(gump);
}

void SourceDebugger::takeControl(void)
{
	Dispatch(new DebugGump(NewGumpId(0), 0, Rect(0, 0, 79, 49), this, 0));
}

DebugScroller::DebugScroller(NewGumpId id, NewGump *parent, Rect r, Debugger *d, WindowFrame *frame) :
	Scroller(id, parent, r, 0x1e, frame)
{
	SetArea(__FILE__, 140);	// __LINE__
	cursorX = 4;
	cursorY = 0;
	registerHotKeys(0x13c, 0x148, 0x149, 0x150, 0x151, 0x14d, 0x14b, 0x147, 0x14f, 0);
	owner = d;
	maxX = 0x100;
	maxY = viewFile->numLines;
	cursorGotoOrigin();
}

void DebugScroller::cursorGotoOrigin(void)
{
	cursorX = 4;
	cursorY = owner->currentLine - 1;
	trackCursor();
}

void DebugScroller::scrollTo(int line)
{
	cursorX = 0;
	cursorY = line;
	trackCursor();
	refresh();
}

void DebugScroller::commandKeyboard(Event &event)
{
	SetArea(__FILE__, 177);	// __LINE__
	int height = rect.height();
	int width = rect.width();
	int oldX = scrollX;
	int oldY = scrollY;
	switch (event.data)
	{
	case 0x13c:	// F2
		if (owner->isBreakPointAt(getSourceFile(), cursorY))
			owner->removeBreakPoint(getSourceFile(), cursorY);
		else
			owner->addBreakPoint(getSourceFile(), cursorY);
		refresh();
		break;
	case 0x148:	// up
		if (cursorY > 0)
			cursorY--;
		break;
	case 0x149:	// page up
		cursorY -= height;
		scrollY -= height;
		break;
	case 0x150:	// down
		cursorY++;
		break;
	case 0x151:	// page down
		cursorY += height;
		scrollY += height;
		break;
	case 0x14d:	// right
		cursorX++;
		break;
	case 0x14b:	// left
		cursorX--;
		break;
	case 0x147:	// home
		cursorX = 0;
		break;
	case 0x14f:	// end
		cursorX = parseLine(WorkString, viewFile->getLine(cursorY), 80) + 4;
		break;
	}
	trackCursor();
	if (scrollX != oldX || scrollY != oldY)
		refresh();
}

// Copies a source line with tabs expanded; returns its length.
int DebugScroller::parseLine(char *dest, char *src, int length)
{
	char *start = dest;
	memset(dest, ' ', length - 1);
	int column = 0;
	while (*src && column < length)
	{
		if (*src == '\t')
		{
			dest += 4;
			column += 4;
			src++;
		}
		else
		{
			*dest++ = *src++;
			column++;
		}
	}
	start[length - 1] = 0;
	return dest - start;
}

void DebugScroller::draw(short, short)
{
	char marker;
	int height;
	int numLines;
	int current;
	int brk;
	int brkLine;
	int row;
	int line;
	char text[80];

	SetArea(__FILE__, 291);	// __LINE__
	memset(text, ' ', sizeof text);
	text[sizeof text - 1] = 0;
	height = rect.height();
	numLines = viewFile->numLines;
	current = owner->currentLine;
	brk = owner->getNextBreak(getSourceFile(), scrollY);
	if (brk != -1)
		brkLine = owner->breaks[brk].line;
	else
		brkLine = -1;
	line = scrollY;
	for (row = 0; row < height; row++)
	{
		gotoxy(0, row);
		if (line < numLines)
		{
			parseLine(text, viewFile->getLine(line), rect.f_04);
			if (current - 1 == line)
				marker = 0x10;
			else
				marker = ' ';
			if (line == brkLine)
			{
				setColor(0x4f);
				if (owner->numBreaks - 1 > brk)
					brkLine = owner->breaks[++brk].line;
			}
			else
				setColor(0x1e);
		}
		else
		{
			marker = ' ';
			memset(text, ' ', sizeof text);
			text[sizeof text - 1] = 0;
		}
		print(" %c  %s", marker, text);
		line++;
	}
	trackCursor();
}

// Keeps the cursor inside the file and scrolls to keep it in view.
void DebugScroller::trackCursor(void)
{
	int height = rect.height();
	int width = rect.width();
	if (cursorY < 0)
		cursorY = 0;
	if (cursorY > maxY)
		cursorY = maxY;
	if (cursorX < 4)
		cursorX = 4;
	if (cursorX > maxX)
		cursorX = maxX;
	if (scrollY < 0)
		scrollY = 0;
	if (scrollY > maxY)
		scrollY = maxY;
	if (cursorY < scrollY)
		scrollY = cursorY;
	if (cursorY >= scrollY + height)
		scrollY = cursorY - height + 15;
	if (cursorX < scrollX)
		scrollX = cursorX;
	if (cursorX >= scrollX + width)
		scrollX = cursorX - width + 1;
	int x = get_dx() + cursorX - scrollX;
	int y = get_dy() + cursorY - scrollY;
	moveCursor(x, y);
}

unsigned char DebugScroller::setFile(char *file)
{
	scrollY = cursorY = scrollX = 0;
	cursorX = 4;
	if (viewFile->setFile(file))
	{
		maxY = viewFile->numLines;
		return 1;
	}
	return 0;
}

DebugGump::DebugGump(NewGumpId id, NewGump *parent, Rect r, Debugger *d, unsigned char fromInter) :
	WindowGump(id, parent, r, 0, 1, 0)
{
	theBaseCamera->textVideoMode();
	clear();
	if (!init)
	{
		memset(watches, 0, sizeof watches);
		init++;
	}
	fromInterpreter = fromInter;
	f_2e = 0;
	f_1d |= 0x10;
	sourceFrame = new WindowFrame(NewGumpId(0), this,
		Rect(rect.f_00, rect.f_02 + 1, rect.f_04, rect.f_06 - 15), doubleFrame, 0x1f);
	scroller = new DebugScroller(NewGumpId(0), this,
		Rect(rect.f_00 + 1, rect.f_02 + 2, rect.f_04 - 1, rect.f_06 - 16), d, sourceFrame);
	inputFrame = new WindowFrame(NewGumpId(0), this,
		Rect(rect.f_00, rect.f_06 - 14, rect.f_04, rect.f_06), singleFrame, 0x70);
	input = new WindowGump(NewGumpId(0), this,
		Rect(rect.f_00 + 1, rect.f_06 - 13, rect.f_04 - 1, rect.f_06 - 1), 0x70, 1, inputFrame);
	if (!sourceFrame || !scroller || !inputFrame || !input)
		outOfMemory(__FILE__, 457);	// __LINE__
	input->clear();
	registerHotKeys(0x13d, 0x166, 0x142, 0x141, 0x161, 0x13, 9, 0xe, 0x17, 0x11, 0x164, 6, 0);
	buildMenu();
}

DebugGump::~DebugGump(void)
{
	theBaseCamera->graphicVideoMode();
}

// Reads a line in the input window, after an optional prompt.
void DebugGump::getInput(char *buffer, char *prompt)
{
	char *p;
	scroller->deactivate();
	input->activate();
	input->clear();
	moveCursor(get_dx() + input->rect.f_00, get_dy() + input->rect.f_02);
	if (prompt)
		printf(prompt);
	gets(buffer);
	for (p = buffer; *p && *p != '\r' && *p != '\n'; p++)
		;
	*p = 0;
	input->deactivate();
	scroller->activate();
	refresh();
}

void DebugGump::message(char *text)
{
	scroller->deactivate();
	input->activate();
	input->clear();
	moveCursor(get_dx() + input->rect.f_00, get_dy() + input->rect.f_02);
	printf(text);
	input->deactivate();
	scroller->activate();
	refresh();
}

// Prints the variable `name` of the current routine, or a global flag of that name.
// Row -1 inspects it in full; other rows show it as a watch.
void DebugGump::dumpSymbol(char *, char *data, char *symbols, char *name, int row)
{
	char count;
	int i;
	int bitNumber;
	int numBits;
	char symName[80];

	if (*name == '*')
	{
		switch (toupper(name[1]))
		{
		case 'M':
			printf("coreleft = %ld\n", coreleft());
			return;
		case 'Y':
			Yamm::heyMan();
			printf("Press any key to continue.");
			getch();
			return;
		}
		return;
	}
	count = *symbols++;
	for (i = 0; i < count; i++)
	{
		int isParam = *symbols++;
		int type = *symbols++;
		char offset = *symbols++;
		char size = *symbols++;
		int length = strlen(symbols);
		strncpy(symName, symbols, 80);
		symbols += length + 1;
		long value = 0;
		if (strcmp(symName, name))
			continue;
		switch (type)
		{
		case 'b':
			if (isParam == 0)
				value = *(char *)(data + offset);
			else
				value = *(char *)(*(char **)(data + 6) - offset);
			break;
		case 'i':
		case 'l':
		case 's':
		case 'z':
			if (isParam == 0)
				value = *(int *)(data + offset);
			else
				value = *(int *)(*(char **)(data + 6) - offset);
			break;
		case 'd':
		case 'p':
			if (isParam == 0)
				value = *(long *)(data + offset);
			else
				value = *(long *)(*(char **)(data + 6) - offset);
			break;
		case '$':
			if (isParam == 0)
				value = (long)(data + offset);
			else
				value = (long)(*(char **)(data + 6) - offset);
			break;
		}
		if (row == -1)
		{
			input->clear();
			input->gotoxy(0, 0);
			input->print("%s", name);
			input->gotoxy(0, 1);
			for (int j = 0; j < 9; j++)
			{
				if (symbolTypes[j].type == type)
				{
					input->print("%s", symbolTypes[j].name);
					break;
				}
			}
			input->gotoxy(0, 2);
			if (type == 's')
			{
				input->print("%s", Yamm::resolve(value));
				return;
			}
			if (type == 'l' || type == 'z')
			{
				unsigned short index = 0;
				YammList list(value, 2, 0);
				while (list.traverse(index))
				{
					if (type == 'z')
						input->print("%s", Yamm::resolve(*(unsigned short *)Yamm::resolve(index + 2)));
					else
						for (int k = 2; k < size + 2; k++)
							input->print("%02x ", *(char *)Yamm::resolve(index + k) & 0xff);
					input->print(".");
				}
				return;
			}
			if (type == '$')
			{
				for (int k = 0; k < size; k++)
					input->print("%02x ", *(char *)(value + k) & 0xff);
				return;
			}
			input->print("%ld (0x%lx)", value, value);
			return;
		}
		else
		{
			input->gotoxy(0, row);
			input->print("%s = ", name);
			if (type == 's')
			{
				input->print("%s", Yamm::resolve(value));
				return;
			}
			if (type == 'l' || type == 'z')
			{
				unsigned short index = 0;
				YammList list(value, 2, 0);
				while (list.traverse(index))
				{
					if (type == 'z')
						input->print("%s", Yamm::resolve(*(unsigned short *)Yamm::resolve(index + 2)));
					else
						for (int k = 2; k < size + 2; k++)
							input->print("%02x ", *(char *)Yamm::resolve(index + k) & 0xff);
					input->print(".");
				}
				return;
			}
			if (type == '$')
			{
				for (int k = 0; k < size; k++)
					input->print("%02x ", *(char *)(value + k) & 0xff);
				return;
			}
			input->print("%ld (0x%lx)", value, value);
			return;
		}
	}
	if (GlobalFlag::findFlag(name, bitNumber, numBits))
	{
		input->clear();
		input->gotoxy(0, 0);
		input->print("%s", name);
		input->gotoxy(0, 1);
		input->print("GLOBAL FLAG, bit number %d, num bits %d", bitNumber, numBits);
		input->gotoxy(0, 2);
		input->print("%d", GlobalFlag::get(bitNumber, numBits));
	}
}

void DebugGump::draw(short, short)
{
	if (debugger->currentInfo)
	{
		Info *info = debugger->getCurrentInfo();
		if (info)
		{
			int row = 0;
			for (int i = 0; i < NUM_WATCHES; i++)
				if (watches[i])
				{
					dumpSymbol(info->process, info->data, info->symbols, watches[i], row);
					row++;
				}
		}
	}
}

void DebugGump::commandKeyboard(Event &event)
{
	switch (event.data)
	{
	case 0x13d:	// F3
		event.data = CMD_LOAD_UNIT;
		break;
	case 0x166:	// ctrl-F9
		event.data = CMD_RUN;
		break;
	case 0x142:	// F8
		event.data = CMD_STEP_OVER;
		break;
	case 0x141:	// F7
		event.data = CMD_TRACE_INTO;
		break;
	case 9:
	case 0x161:
		event.data = CMD_INSPECT;
		break;
	case 0x17:
	case 0x164:
		event.data = CMD_WATCH;
		break;
	case 0x13:
		event.data = CMD_FIND;
		break;
	case 0xe:
		event.data = CMD_SEARCH_AGAIN;
		break;
	case 0x11:
		event.data = CMD_CLEAR_WATCH;
		break;
	case 6:
		event.data = CMD_CHANGE_FLAG;
		break;
	}
	doMenuCommand(event);
}

void DebugGump::commandPrivate(Event &event)
{
	switch (event.from->newGumpId.getInstance())
	{
	case 1:
		doMenuCommand(event);
	}
}

void DebugGump::doMenuCommand(Event &event)
{
	Info *info = debugger->getCurrentInfo();
	int i;

	switch (event.data)
	{
		int bitNumber;
		int numBits;
		int value;
		int max;
		int old;
		char *format;
		int skip;
		char buffer[80];
		char prompt[256];

	case CMD_RUN:
		debugger->steppingOver = 0;
		debugger->tracing = 0;
		theBaseCamera->graphicVideoMode();
		killMyself();
		return;
	case CMD_STEP_OVER:
		debugger->stepOver();
		killMyself();
		return;
	case CMD_TRACE_INTO:
		debugger->steppingOver = 0;
		debugger->tracing = 1;
		killMyself();
		return;
	case CMD_LOAD_UNIT:
		getInput(buffer, 0);
		if (!scroller->setFile(buffer))
			message("unable to open file");
		refresh();
		return;
	case CMD_BREAK:
		asm int 3;
		refresh();
		return;
	case CMD_INSPECT:
		getInput(buffer, 0);
		dumpSymbol(info->process, info->data, info->symbols, buffer, -1);
		return;
	case CMD_WATCH:
		getInput(buffer, 0);
		for (i = 0; i < NUM_WATCHES; i++)
		{
			if (!watches[i])
			{
				watches[i] = new char[strlen(buffer) + 1];
				strcpy(watches[i], buffer);
				return;
			}
		}
		return;
	case CMD_CLEAR_WATCH:
		for (i = 0; i < NUM_WATCHES; i++)
		{
			if (watches[i])
			{
				delete watches[i];
				watches[i] = 0;
			}
		}
		return;
	case CMD_FIND:
		getInput((char *)searchBuffer, "Search For:");
		for (i = 0; i < DebugScroller::viewFile->getNumLines(); i++)
		{
			if (strstr(DebugScroller::viewFile->getLine(i), (char *)searchBuffer))
			{
				scroller->scrollTo(i);
				return;
			}
		}
		return;
	case CMD_SEARCH_AGAIN:
		for (i = scroller->cursorY + 1; i < DebugScroller::viewFile->getNumLines(); i++)
		{
			if (strstr(DebugScroller::viewFile->getLine(i), (char *)searchBuffer))
			{
				scroller->scrollTo(i);
				return;
			}
		}
		return;
	case CMD_CHANGE_FLAG:
		getInput(buffer, "Flag name:");
		if (GlobalFlag::findFlag(buffer, bitNumber, numBits))
		{
			max = 1 << (numBits - 1);
			old = GlobalFlag::get(bitNumber, numBits);
			sprintf(prompt, "Old Value: (%d)   [0x%04x]  New Value: (0-%d) [0x0-0x%04x]:", old, old, max, max);
			getInput(buffer, prompt);
			if (strlen(buffer) == 0)
				return;
			skip = 0;
			if (buffer[1] == 'x' || buffer[1] == 'X')
			{
				format = "%x";
				skip = 2;
			}
			else
				format = "%d";
			sscanf(buffer + skip, format, &value);
			if (value >= 0 && value <= max)
			{
				GlobalFlag::set(bitNumber, numBits, value);
				message("Done!");
				return;
			}
			message("Out of Range!");
			return;
		}
		message("This flag does not exist!");
	}
}

void DebugGump::buildMenu(void)
{
	TextMenuBarGump &bar = *new TextMenuBarGump(1, this)
		+ (*new TextMenuItemGump("File", 0, 1)
			+ (*new TextMenuItemGump("Load Unit... F3", CMD_LOAD_UNIT, 1)
			+ *new TextMenuItemGump("View File...", CMD_VIEW_FILE, 1)
			+ *new TextMenuItemGump("Break to TDP", CMD_BREAK, 1)))
		+ (*new TextMenuItemGump("Run", 0, 1)
			+ (*new TextMenuItemGump("Run              Ctl-F9", CMD_RUN, 1)
			+ *new TextMenuItemGump("Run to cursor        F4", CMD_RUN_TO_CURSOR, 1)
			+ *new TextMenuItemGump("Trace into           F7", CMD_TRACE_INTO, 1)
			+ *new TextMenuItemGump("Step over            F8", CMD_STEP_OVER, 1)
			+ *new TextMenuItemGump("Run until return Alt-F8", CMD_RUN_TO_RETURN, 1)
			+ *new TextMenuLineGump
			+ *new TextMenuItemGump("Breakpoint           F2", CMD_BREAKPOINT, 1)))
		+ (*new TextMenuItemGump("Search", 0, 1)
			+ (*new TextMenuItemGump("Find...", CMD_FIND, 1)
			+ *new TextMenuItemGump("Search again", CMD_SEARCH_AGAIN, 1)
			+ *new TextMenuLineGump
			+ *new TextMenuItemGump("Go to line number...", CMD_GOTO_LINE, 1)))
		+ (*new TextMenuItemGump("Data", 0, 1)
			+ (*new TextMenuItemGump("Watch", CMD_WATCH, 1)
			+ *new TextMenuItemGump("Inspect...", CMD_INSPECT, 1)
			+ *new TextMenuItemGump("Clear Watch", CMD_CLEAR_WATCH, 1)
			+ *new TextMenuItemGump("Change Flag", CMD_CHANGE_FLAG, 1)));
	bar.moveto(0, 0);
	bar.rect.f_04 = 79;
	bar.refresh();
}

ViewFile::ViewFile(void)
{
	text = 0;
	numLines = 0;
	memset(name, 0, sizeof name);
}

unsigned char ViewFile::setFile(char *file)
{
	char *p = strstr(file, "\r");
	if (p)
		*p = 0;
	if (text)
	{
		farfree(text);
		text = 0;
	}
	numLines = 0;
	if (!file)
		return 1;
	FileSpec spec(file, 0);
	if (!spec.dir[0])
		strcpy(spec.dir, UsecodeDir);
	if (!spec.ext[0])
		strcpy(spec.ext, ".unk");
	strcpy(name, spec.name);
	char *path = (char *)spec.get(1);
	if (!spec.exists())
		return 0;
	if (filename && !strcmp(path, filename))
		return 1;
	open(path, ReadOnly);
	loadFile();
	close();
	return 1;
}

void ViewFile::loadFile(void)
{
	long length = size();
	text = (char *)farmalloc(length + 1);
	if (!text)
		outOfMemory(__FILE__, 1030);	// __LINE__
	read(text, length);
	Desc *desc = LocalDescriptorTable::getDesc(FP_SEG(text));
	unsigned long base = LocalDescriptorTable::getDesc(FP_SEG(text))->getBase();
	numLines = parse(lines, text, length, base);
	if (numLines >= 4000)
		halt(__FILE__, 1062);	// __LINE__
}

char *ViewFile::getLine(unsigned short line)
{
	return (char *)makePtr(lines[line]);
}
