// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: EVENT\CTMPOINT.C

#include "CEXIT.H"
#include "CCURRVP.H"
#include "CMPOINT.H"
#include "CMSELECT.H"
#include "CSHAPE.H"
#include "CTMPOINT.H"
#include "CVPORT.H"
#include "CMEMHNDP.H"

// Inline in the shared headers when this file was built.
inline void GraphicPoint::set_position(short x, short y) { f_02 = x; f_04 = y; }
inline void Shape::set_frame(short frame) { f_18 = frame; }
inline Point::Point(void) {}

Vport *TMMousePointer::mapped_vport = 0;
TMMousePointer *theMouse = 0;
Shape *TMMousePointer::pointer;
Point TMMousePointer::dimensions;

// There is only one pointer; theMouse finds it for getFrame.
TMMousePointer::TMMousePointer(void)
{
	dimensions.f_00 = 0;
	dimensions.f_02 = 0;
	mapped_vport = GlobalVport::main_screen;
	old_x = old_y = 0;
	if (theMouse)
		halt(__FILE__, 54);
	theMouse = this;
}

// Unless told not to, takes the pointer off the old viewport first.
void TMMousePointer::map_to(Vport &v, unsigned char quietly)
{
	unsigned char wasVisible;

	pushCat(CAT_BUSY);
	if (!quietly)
	{
		if ((wasVisible = visible) != 0)
			hide();
		mapped_vport = &v;
		if (wasVisible)
			show();
	}
	else
		mapped_vport = &v;
	popCat();
}

// Mouse driver callback; x comes in doubled for the 320-pixel screen.
void TMMousePointer::handle(int, int, int x, int y)
{
	if (visible && cat[catSP] == CAT_FREE)
	{
		pushCat(CAT_BUSY);
		undraw();
		draw(x >> 1, y);
		popCat();
	}
	else
	{
		old_x = x >> 1;
		old_y = y;
	}
}

void TMMousePointer::set_shape(MemHandle &handle)
{
	pushCat(CAT_BUSY);
	if (!pointer)
		pointer = new Shape;
	pointer->set_shape(handle);
	popCat();
}

void TMMousePointer::show(void)
{
	pushCat(CAT_BUSY);
	if (visible)
		undraw();
	draw(old_x, old_y);
	visible = 1;
	popCat();
}

void TMMousePointer::hide(void)
{
	pushCat(CAT_BUSY);
	if (visible)
		undraw();
	visible = 0;
	popCat();
}

void TMMousePointer::draw(int x, int y)
{
	CurrentVport current(mapped_vport);

	pointer->set_frame(theMouse->getFrame(x, y));
	pointer->set_position(x, y);
	pointer->hide();
	pointer->draw(x, y);
	old_x = x;
	old_y = y;
}

void TMMousePointer::undraw(void)
{
	CurrentVport current(mapped_vport);

	pointer->restore(old_x, old_y);
}

void TMMousePointer::draw(Vport &v)
{
	Vport *saved;

	pushCat(CAT_BUSY);
	saved = mapped_vport;
	mapped_vport = &v;
	draw(old_x, old_y);
	mapped_vport = saved;
	popCat();
}

void TMMousePointer::undraw(Vport &v)
{
	Vport *saved;

	pushCat(CAT_BUSY);
	saved = mapped_vport;
	mapped_vport = &v;
	undraw();
	mapped_vport = saved;
	popCat();
}

// Draws the pointer into v before it is copied to the screen; the pointer
// stays locked until after_slam takes it off again.
void TMMousePointer::before_slam(Vport *v)
{
	pushCat(CAT_BUSY);
	draw(*v);
}

void TMMousePointer::after_slam(Vport *v)
{
	undraw(*v);
	popCat();
}

inline int TMMousePointer::getFrame(int, int)
{
	return 0;
}
