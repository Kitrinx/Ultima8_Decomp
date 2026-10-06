// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -vi-
// name: DRAGGUMP.C

#include "..\UI\NEWGUMP.H"
#include "BASECAM.H"
#include "CDESC.H"
#include "CVPORT.H"
#include "CCURRVP.H"
#include "CSHAPE.H"
#include "FWRITSKP.H"
#include "CEXIT.H"
#include "DISPATCH.H"
#include "MENU.H"
#include "DRAGGUMP.H"
#include "CMEMHNDP.H"

// Inline in the shared headers when this file was built.
inline Rect::Rect(void) {}
inline void Point::set(short x, short y) { f_00 = x; f_02 = y; }
inline int Rect::width(void) { return f_04 - f_00 + 1; }
inline int Rect::height(void) { return f_06 - f_02 + 1; }
inline void NewGump::popVisibility(void) { visibilityStack >>= 2; }
inline GraphicYtable::GraphicYtable(void) { f_04 = 2; index = 0; }
inline Vport::Vport(void) { seg = 0; f_0f = 2; f_10 = 0; }
inline Vport::~Vport(void) { free(); }
inline Rect *Vport::getRect(void) { return &rect; }
inline void MemHandle::init(char *p, unsigned char owned) { f_00 = (unsigned long)p; f_04 = owned; f_05 = 0; }
inline MemHandle::MemHandle(void) { init(0, 0); }
inline MemHandle::~MemHandle(void) { free(0); }
inline char *MemHandle::get_ptr(void) { return (char *)f_00; }
inline void BaseCamera::setNeedSlam(void) { needSlam = 1; }

// A one-frame shape built in memory: the header, then the frame table.
struct DragShapeHeader
{
	int width;
	int height;
	int frames;
	long offset;
	int size;
};

struct DragFrameHeader
{
	int f_00;
	int f_02;
	long f_04;
};

DrgNewGump::DrgNewGump(NewGumpId id, NewGump *parent) :
	NewGump(id, parent, 2)
{
	dragShape = new Shape;
	if (!dragShape)
		outOfMemory(__FILE__, 42);	// __LINE__ in the original file
	dragging = 0;
}

DrgNewGump::~DrgNewGump(void)
{
	if (dragShape)
	{
		delete dragShape;
		dragShape = 0;
	}
}

// A drag draws the gump once into a shape that then follows the mouse.
void DrgNewGump::commandMouseLeft(Event &event)
{
	if (dragging == 0 && event.isSingle())
	{
		dragging = 1;
		lastX = event.x;
		lastY = event.y;
		f_1d |= 8;
		setExclusive(this);
		pushVisibility(3);
		restore(rect);
		popVisibility();
		Rect saved = rect;
		rect.moveto(0, 0);
		Vport vport;
		*vport.getRect() = rect;
		if (!vport.alloc(0))
			halt(__FILE__, 105);	// __LINE__ in the original file
		vport.clear(GumpColorMap[7]);
		MemHandle mem;
		{
			CurrentVport current(&vport);
			refresh(0, 0, *vport.getRect());
			pushVisibility(3);
			unsigned size = EstimateFastWriteSkip(vport, rect.f_00, rect.f_02, rect.f_04, rect.f_06, GumpColorMap[7]);
			if (size == -1)
				halt(__FILE__, 128);	// __LINE__ in the original file
			int headerSize = 20;
			mem.alloc(size + headerSize, 2);
			int written = FastWriteSkip(vport, rect.f_00, rect.f_02, rect.f_04, rect.f_06, 0, 0, (unsigned char *)mem.get_ptr() + headerSize, GumpColorMap[7]);
			if (size < written)
				halt(__FILE__, 141);	// __LINE__ in the original file
			DragShapeHeader *header = (DragShapeHeader *)mem.get_ptr();
			header->width = rect.width();
			header->height = rect.height();
			header->frames = 1;
			header->offset = 12;
			header->size = written;
			DragFrameHeader *frame = (DragFrameHeader *)(mem.get_ptr() + 12);
			frame->f_00 = -1;
			frame->f_02 = -1;
			frame->f_04 = -1;
		}
		rect = saved;
		dragShape->set_shape(mem);
		((Point *)&dragShape->f_02)->set(get_dx(), get_dy());
		dragShape->hide();
		commandMouseMovement(event);
		return;
	}
	if (dragging && event.isRelease())
	{
		dragging = 0;
		clrExclusive();
		f_1d &= ~8;
		dragShape->free();
		if (parent)
			parent->newGumpList.moveToTail(this);
		popVisibility();
		refresh();
	}
}

void DrgNewGump::commandMouseMovement(Event &event)
{
	if (dragging)
	{
		int dx = event.x - lastX;
		int dy = event.y - lastY;
		dragShape->move(dx, dy);
		rect.moverel(dx, dy);
		BaseCamera::setNeedSlam();
		lastX = event.x;
		lastY = event.y;
	}
}
