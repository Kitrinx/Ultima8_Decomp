// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: TBUTTON.C

#include "DISPATCH.H"
#include "BASECAM.H"
#include "FONT.H"
#include "TBUTTON.H"

TextPushButton::TextPushButton(unsigned char id, NewGump *parent, char *text, unsigned char color) :
	PushButton(id, parent)
{
	this->text = text;
	int width, height;
	GlobalFont::global_ptr->Font::dim(this->text, width, height);
	rect = Rect(0, 0, width + 8, height + 4);
	_frame = 0;
	this->color = color;
}

void TextPushButton::draw(short x, short y)
{
	Panel panel;
	Rect r(x, y, x + rect.width(), y + rect.height());
	unsigned char c = enabled ? color : color + 2;
	if (_frame == 0)
		panel.drawup(r, c);
	else
		panel.drawdown(r, c);
	int height = GlobalFont::global_ptr->Font::height(text);
	GlobalFont::global_ptr->printf(x + _frame + 4, y + height + _frame + 2, text);
}

void TextPushButton::select(void)
{
	set_frame(1);
}

void TextPushButton::unselect(void)
{
	set_frame(0);
}

TextActivatorButton::TextActivatorButton(unsigned char id, NewGump *parent, char *text, unsigned char color) :
	ActivatorButton(id, parent)
{
	this->text = text;
	int width, height;
	GlobalFont::global_ptr->Font::dim(this->text, width, height);
	rect = Rect(0, 0, width + 8, height + 4);
	_frame = 0;
	this->color = color;
}

void TextActivatorButton::draw(short x, short y)
{
	Panel panel;
	Rect r(x, y, x + rect.width(), y + rect.height());
	unsigned char c = enabled ? color : color + 2;
	if (_frame == 0)
		panel.drawup(r, c);
	else
		panel.drawdown(r, c);
	int height = GlobalFont::global_ptr->Font::height(text);
	GlobalFont::global_ptr->printf(x + _frame + 4, y + height + _frame + 2, text);
}

void TextActivatorButton::select(void)
{
	set_frame(1);
}

void TextActivatorButton::unselect(void)
{
	set_frame(0);
}

inline int TextPushButton::set_frame(int frame)
{
	_frame = frame;
	refresh();
}

inline int TextActivatorButton::set_frame(int frame)
{
	_frame = frame;
	refresh();
}
