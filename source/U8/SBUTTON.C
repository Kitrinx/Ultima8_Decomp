// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: SBUTTON.C

#include "CRECT.H"
#include "DISPATCH.H"
#include "FVINFO.H"
#include "SBUTTON.H"
#include "U8GMPSHP.H"

// Inline in the shared headers when this file was built.
inline GumpShape::GumpShape(int shape) { _frame = 0; this->shape = shape; }
inline Rect GumpShape::get_rect(void) { return drawer->get_rect(shape, _frame); }
inline void GumpShape::draw(int x, int y) { drawer->draw(x, y, shape, _frame); }
inline void *GumpShape::get(void) { return drawer->get(shape); }
inline void GumpShape::set_frame(int frame) { _frame = frame; }
inline int ShapePushButton::set_frame(int frame) { GumpShape::set_frame(frame); refresh(); }
inline int ShapeCycleButton::set_frame(int frame) { GumpShape::set_frame(frame); refresh(); CycleButton::_frame = frame; }
inline int ShapeActivatorButton::set_frame(int frame) { GumpShape::set_frame(frame); refresh(); }
inline void ShapeCycleButton::unselect(void) { set_frame(CycleButton::_frame); }

ShapePushButton::ShapePushButton(unsigned char id, NewGump *parent, int shape) :
	PushButton(id, parent), GumpShape(shape)
{
	rect = get_rect();
}

void ShapePushButton::draw(short x, short y)
{
	GumpShape::draw(x, y);
}

void ShapePushButton::select(void)
{
	set_frame(1);
}

void ShapePushButton::unselect(void)
{
	set_frame(0);
}

ShapeCycleButton::ShapeCycleButton(unsigned char id, NewGump *parent, int shape) :
	CycleButton(id, parent), GumpShape(shape)
{
	f_37 = GetNumShapes(get());
	rect = get_rect();
}

void ShapeCycleButton::draw(short x, short y)
{
	GumpShape::draw(x, y);
}

// Each state has a lit and an unlit frame.
void ShapeCycleButton::select(void)
{
	set_frame((CycleButton::_frame + 3) % (f_37 * 2));
}

ShapeActivatorButton::ShapeActivatorButton(unsigned char id, NewGump *parent, int shape) :
	ActivatorButton(id, parent), GumpShape(shape)
{
	rect = get_rect();
}

void ShapeActivatorButton::draw(short x, short y)
{
	GumpShape::draw(x, y);
}

void ShapeActivatorButton::select(void)
{
	set_frame(1);
}

void ShapeActivatorButton::unselect(void)
{
	set_frame(0);
}
