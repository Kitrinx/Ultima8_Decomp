// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: COMBOBOX.C

#include <string.h>
#include "..\UI\NEWGUMP.H"
#include "COMBOBOX.H"
#include "CRECT.H"
#include "DISPATCH.H"
#include "ERROR.H"
#include "LISTBOX.H"
#include "TEXTEDIT.H"

inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline void NewGump::moveto(short x, short y) { rect.moveto(x, y); }
inline unsigned char NewGumpId::getType(void) { return instance >> 8; }
inline int ListBoxGump::getSelected(void) { return selected; }

ComboBox::ComboBox(unsigned char id, NewGump *parent, Rect r, int len) :
	NewGump(NewGumpId(8, id), parent, EVENT_PRIVATE)
{
	rect = r;
	text = new char[len];
	*text = 0;
	lastText = new char[len];
	*lastText = 0;
	if (!text || !lastText)
		outOfMemory(__FILE__, 72);	// __LINE__
	list = 0;
	textEdit = new TextEditGump(0, this, text, len, rect.width(), 0, 0);
	textEdit->f_1d |= 0x20;
	Rect listRect(0, 0, rect.width(), rect.height() - textEdit->rect.height());
	listBox = new ListBoxGump(0, this, listRect, list, 1, -1, -1);
	listBox->moveto(0, textEdit->rect.f_06);
}

ComboBox::~ComboBox(void)
{
	delete text;
	delete lastText;
}

void ComboBox::commandPrivate(Event &event)
{
	switch (event.from->newGumpId.getType())
	{
	case 6:
		if (event.data == 2 || event.data == 3)
		{
			if (!*list)
				return;
			int index = 0;
			int cmp;
			while ((cmp = strnicmp(text, list[index], strlen(text))) != 0 && list[index + 1])
				index++;
			if (cmp)
				strcpy(text, lastText);
			else
			{
				strcpy(lastText, text);
				listBox->setSelected(index);
			}
			textEdit->reset(0, 0);
			refresh();
			send(Event(EVENT_PRIVATE, listBox->getSelected(), 0, 1, this, parent, 0));
		}
		else
		{
			int selected = listBox->getSelected();
			strcpy(text, selected < 0 ? "" : list[selected]);
			refresh();
			send(Event(EVENT_PRIVATE, listBox->getSelected(), 0, 2, this, parent, 0));
		}
		break;
	case 5:
		if (event.data == 2)
		{
			int selected = listBox->getSelected();
			strcpy(text, selected < 0 ? "" : list[selected]);
			refresh();
			send(Event(EVENT_PRIVATE, listBox->getSelected(), 0, 2, this, parent, 0));
		}
		else
		{
			int selected = listBox->getSelected();
			strcpy(text, selected < 0 ? "" : list[selected]);
			textEdit->reset(1, 0);
			refresh();
			send(Event(EVENT_PRIVATE, listBox->getSelected(), 0, 1, this, parent, 0));
		}
		break;
	}
}

char *ComboBox::getSelected(void)
{
	int selected = listBox->getSelected();
	char *s = 0;
	if (selected != -1)
		s = list[listBox->getSelected()];
	return s;
}
