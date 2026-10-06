// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: FILEBOX.C

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <mem.h>
#include <dir.h>
#include <dos.h>
#include <direct.h>
#include "..\UI\NEWGUMP.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "ERROR.H"
#include "MENU.H"
#include "TBUTTON.H"
#include "FILEBOX.H"
#include "FILESPEC.H"
#include "TEXTEDIT.H"

// Inline in the shared headers when this file was built.
inline Rect::Rect(short x1, short y1, short x2, short y2) { set(x1, y1, x2, y2); }
inline void NewGump::moveto(short x, short y) { rect.moveto(x, y); }
inline unsigned char NewGumpId::getType(void) { return instance >> 8; }
inline unsigned char NewGumpId::getInstance(void) { return instance & 0xff; }
inline int ListBoxGump::getSelected(void) { return selected; }
inline char *TextEditGump::getData(void) { return text; }
inline FileSpec::FileSpec(char *path, int attrib) { become(path, attrib); }
inline char *FileSpec::get(int full) { if (full) return fileSpec(drive, dir, name, ext); return fileSpec(0, 0, name, ext); }
inline char *FileBox::getDesc(void) { return fileSpec; }
inline NewGump *NewGumpList::hasType(unsigned char type) { NewGump *gump = 0; return hasType(type, gump); }

// The file list and its text field.
FileBox::FileBox(NewGump *parent, char *spec) :
	ComboBox(3, parent, Rect(0, 0, 85, 119), 80)
{
	setFileSpec(spec);
	textEdit->rect = Rect(0, 0, 171, 12);
}

void FileBox::setFileSpec(char *spec)
{
	if (spec)
		strcpy(fileSpec, spec);
	strcpy(text, fileSpec);
	init();
	textEdit->reset(1, 0);
}

void FileBox::init(void)
{
	if (list)
		unInit();
	FileSpec spec;
	spec.become(fileSpec, 0);
	if (spec.name[0] == 0)
		return;
	int count = 1;
	while (spec.getNext())
		count++;
	if (!count)
		return;
	list = new char *[count + 1];
	if (!list)
		outOfMemory(__FILE__, 155);	// __LINE__ in the original file
	if (count >= 1)
	{
		count = 0;
		spec.getFirst();
		do
		{
			list[count] = new char[13];
			sprintf(list[count], "%s%s", spec.name, spec.ext);
			count++;
		} while (spec.getNext());
	}
	else
		count = 0;
	list[count] = 0;
	listBox->setList(list, 1, -1, -1);
	listBox->setSelected(-1);
}

void FileBox::unInit(void)
{
	if (list)
	{
		char *name = list[0];
		int i = 1;
		while (name)
		{
			delete name;
			name = list[i++];
		}
		delete list;
		list = 0;
		listBox->setList(list, 1, -1, -1);
	}
}

void FileBox::commandPrivate(Event &event)
{
	switch (event.from->newGumpId.getType())
	{
	case 6:		// the text field
		if (event.x == 13)
		{
			DirectoryBox *dirBox = (DirectoryBox *)parent->newGumpList.hasInstance(4);
			text = textEdit->getData();
			if (dirBox->setDir(text))
			{
				setFileSpec(text);
				parent->refresh();
				return;
			}
		}
		else if ((unsigned)event.x >= 0x80)
		{
			NewGump::commandKeyboard(event);
			return;
		}
		break;
	case 5:		// the list
		if (event.data == 2)
			send(Event(EVENT_PRIVATE, 0, 0, 5, this, parent, 0));
	}
}

void FileBox::setDrive(char drive)
{
	char cwd[80];
	_getdcwd(drive - 64, cwd, 80);
	FileSpec spec(cwd, 0);
	strcpy(spec.name, "*");
	strcpy(spec.ext, ".*");
	strcpy(fileSpec, spec.get(1));
	init();
	listBox->setSelected(0);
	strcpy(text, fileSpec);
	textEdit->reset(1, 0);
}

void FileBox::setWild(char *wild)
{
	FileSpec spec(fileSpec, 0);
	FileSpec pattern(wild, 0);
	strcpy(spec.name, pattern.name);
	strcpy(spec.ext, pattern.ext);
	strcpy(fileSpec, spec.get(1));
	init();
	listBox->setSelected(0);
	strcpy(text, fileSpec);
	textEdit->reset(1, 0);
}

// The directory list; double clicking moves into or up out of one.
DirectoryBox::DirectoryBox(NewGump *parent, char *path) :
	ListBoxGump(4, parent, Rect(0, 0, 85, 107), 0, 1, -1, -1)
{
	dirs = 0;
	if (!setDir(path))
		setDir(".");
}

unsigned char DirectoryBox::setDir(char *path)
{
	FileSpec spec(path, 0);
	strcpy(spec.name, "*");
	strcpy(spec.ext, ".*");
	if (dirs)
	{
		char *name = dirs[0];
		int i = 1;
		while (name)
		{
			delete name;
			name = dirs[i++];
		}
		delete dirs;
		dirs = 0;
		setList(dirs, 1, -1, -1);
	}
	spec.become(spec.get(1), FA_DIREC);
	if (spec.name[0] == 0)
		return 0;
	int count = 0;
	while (spec.getNext())
		count++;
	if (count)
	{
		dirs = new char *[count + 1];
		if (!dirs)
			outOfMemory(__FILE__, 329);	// __LINE__ in the original file
		count = 0;
		spec.getFirst();
		while (spec.getNext())
		{
			dirs[count] = new char[13];
			if (spec.ext[0])
				sprintf(dirs[count], "%s%s", spec.name, spec.ext);
			else
				sprintf(dirs[count], "%s", spec.name);
			count++;
		}
		dirs[count] = 0;
		setList(dirs, 1, -1, -1);
		setSelected(-1);
	}
	return 1;
}

void DirectoryBox::commandMouseLeft(Event &event)
{
	ListBoxGump::commandMouseLeft(event);
	if (event.isDouble())
	{
		char name[80];
		strcpy(name, dirs[getSelected()]);
		FileBox *fileBox = (FileBox *)parent->newGumpList.hasInstance(3);
		FileSpec spec(fileBox->getDesc(), 0);
		if (!strcmp(name, ".."))
		{
			int len = strlen(spec.dir) - 1;
			if (!len)
				return;
			while (spec.dir[--len] != '\\' && len)
				;
			spec.dir[len + 1] = 0;
		}
		else
		{
			strcat(spec.dir, name);
			strcat(spec.dir, "\\");
		}
		spec.reset();
		fileBox->setFileSpec(spec.get(1));
		setDir(spec.get(1));
		parent->refresh();
	}
}

// A file chooser: file list, directory list, drive buttons, and the caller's
// buttons named in the zero-terminated argument list.
FileSelector::FileSelector(NewGumpId id, NewGump *parent, char *path, FileSpec *spec, ...) :
	PanelGump(id, parent, Rect(0, 0, 280, 150), GumpColorMap[5])
{
	va_list ap;
	char *name;
	int i;
	TextPushButton *button;
	int y;
	i = 0;
	y = 90;
	memset(names, 0, sizeof(names));
	va_start(ap, spec);
	for (; (name = va_arg(ap, char *)) != 0; i++)
		if (i < 6)
		{
			names[i] = new char[strlen(name) + 1];
			if (!names[i])
				outOfMemory(__FILE__, 420);	// __LINE__ in the original file
			strcpy(names[i], name);
			button = new TextPushButton(i + 32, this, names[i], GumpColorMap[5]);
			button->moveto(i & 1 ? 230 : 190, y);
			if (i & 1)
				y += 18;
		}
	result = spec;
	drive = _getdrive();
	this->path[0] = '\\';
	getcurdir(0, this->path + 1);
	DirectoryBox *dirBox = new DirectoryBox(this, path);
	dirBox->moveto(89, 15);
	FileBox *fileBox = new FileBox(this, path);
	fileBox->moveto(3, 3);
	button = new TextPushButton(1, this, "Open", GumpColorMap[5]);
	button->moveto(3, 127);
	button = new TextPushButton(2, this, "Cancel", GumpColorMap[5]);
	button->moveto(41, 127);
	unsigned saved;
	_dos_getdrive(&saved);
	for (int d = 1; d <= 26; d++)
		if (!_chdrive(d))
		{
			driveNames[d - 1][0] = 'A' + d - 1;
			driveNames[d - 1][1] = 0;
			button = new TextPushButton(d + 5, this, driveNames[d - 1], GumpColorMap[5]);
			button->moveto((d - 1) % 6 * 17 + 176, (d - 1) / 6 * 17 + 3);
		}
	_chdrive(saved);
}

FileSelector::~FileSelector(void)
{
	for (int i = 0; i < 6; i++)
		if (names[i])
			delete names[i];
}

void FileSelector::draw(short x, short y)
{
	PanelGump::draw(x, y);
}

void FileSelector::commandPrivate(Event &event)
{
	unsigned char id = event.from->newGumpId.getInstance();
	FileBox *fileBox = (FileBox *)newGumpList.hasType(8);
	DirectoryBox *dirBox = (DirectoryBox *)newGumpList.hasInstance(4);
	int changed = 0;
	if (id >= 32)
	{
		fileBox->setWild(names[id - 32]);
		dirBox->setDir(fileBox->getDesc());
		changed++;
	}
	if (id >= 6 && id < 32)
	{
		fileBox->setDrive(id - 6 + 'A');
		dirBox->setDir(fileBox->getDesc());
		changed++;
	}
	if (id == 4)
		return;
	if (id != 2)
	{
		if (!buildResult())
			return;
		if (changed)
			refresh();
	}
	if (id == 2 || id == 1 || (id == 3 && event.data == 5))
	{
		send(Event(EVENT_PRIVATE, 0, 0, id != 2, this, parent, 0));
		killMyself();
	}
}

unsigned char FileSelector::buildResult(void)
{
	FileSpec spec;
	FileBox *fileBox = (FileBox *)newGumpList.hasType(8);
	result->become(fileBox->getDesc(), 0);
	char *selected = fileBox->getSelected();
	if (!selected || !strlen(selected))
		return 0;
	spec.become(selected, 0);
	strcpy(result->name, spec.name);
	strcpy(result->ext, spec.ext);
	return 1;
}
