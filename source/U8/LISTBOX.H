#ifndef LISTBOX_H
#define LISTBOX_H

#include "DISPATCH.H"

class Event;
class Rect;

class ListBoxGump : public NewGump
{
public:
	int selected;
	int lineHeight;
	int visibleLines;
	int count;			// -1 until reset() counts a null-terminated list
	unsigned char sorted;
	int stride;			// -1 for an array of string pointers, else the size of one packed string
	char **list;

	ListBoxGump(unsigned char, NewGump *, Rect, char **, unsigned char, int, int);
	virtual void commandNoEvents(Event &);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void commandPrivate(Event &);
	virtual void draw(short, short);
	virtual void sort(void);
	int getSelected(void);
	void setSelected(int);
	void setList(char **, unsigned char, int, int);
	void reset(void);
	char *getString(int);
};

#endif
