#ifndef TBUTTON_H
#define TBUTTON_H

#include "BUTTON.H"

class NewGump;

class TextPushButton : public PushButton
{
public:
	unsigned _frame;
	unsigned char color;
	char *text;

	TextPushButton(unsigned char, NewGump *, char *, unsigned char);
	virtual void draw(short, short);
	virtual void select(void);
	virtual void unselect(void);
	inline int set_frame(int);
};

class TextActivatorButton : public ActivatorButton
{
public:
	unsigned _frame;
	unsigned char color;
	char *text;

	TextActivatorButton(unsigned char, NewGump *, char *, unsigned char);
	virtual void draw(short, short);
	virtual void select(void);
	virtual void unselect(void);
	inline int set_frame(int);
};

#endif
