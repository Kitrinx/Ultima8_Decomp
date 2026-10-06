#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include "DISPATCH.H"

class Event;

// A one-line text field; Tab and Enter pass the focus on.
class TextEditGump : public NewGump
{
public:
	unsigned char lastChar;
	unsigned char len;
	unsigned char maxLen;
	unsigned char selStart;
	unsigned char selEnd;
	unsigned char scroll;		// first character shown
	unsigned char tabNext;		// Tab moves to the next field
	unsigned char cursor;
	char *text;

	TextEditGump(unsigned char, NewGump *, char *, unsigned char, unsigned char, char *, unsigned char);
	virtual ~TextEditGump(void);
	virtual void commandNoEvents(Event &);
	virtual void commandMouseLeft(Event &);
	virtual void commandMouseMovement(Event &);
	virtual void commandKeyboard(Event &);
	virtual void draw(short, short);
	virtual void reset(unsigned char, char *);
	char *getData(void);
	int findLoc(int);
	int findChar(short);
	void findScrollPos(void);
	void getFocus(void);
	void loseFocus(void);
	static TextEditGump *focus;
};

#endif
