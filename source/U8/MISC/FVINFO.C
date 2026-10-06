// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MISC\FVINFO.C

#include "CRECT.H"
#include "CVPORT.H"
#include "FVINFO.H"

inline Point::Point(void) {}
inline Point::Point(short x, short y) { f_00 = x; f_02 = y; }
inline Vport::~Vport(void) { free(); }

// A shape starts with its largest frame width and depth and its frame count,
// then six bytes per frame of which the first word is the frame's offset.
struct ShapeHeader
{
	int maxWidth;
	int maxDepth;
	int numFrames;
};

struct FrameEntry
{
	int offset;
	int unused[2];
};

// Each frame starts with eight bytes before its own header.
#define FRAME_ADDRESS(shape, frame) \
	((char *)(shape) + ((FrameEntry *)((char *)(shape) + sizeof(ShapeHeader)))[frame].offset + 8)

int GetMaxShapeWidth(void *shape)
{
	return ((ShapeHeader *)shape)->maxWidth;
}

int GetMaxShapeDepth(void *shape)
{
	return ((ShapeHeader *)shape)->maxDepth;
}

int GetNumShapes(void *shape)
{
	return ((ShapeHeader *)shape)->numFrames;
}

void *GetShapeAddress(void *shape, int frame)
{
	return FRAME_ADDRESS(shape, frame);
}

void GetShapeInfo(char *shape, short frame, short &width, short &height, short &xOffset, short &yOffset)
{
	int *p = (int *)FRAME_ADDRESS(shape, frame);

	width = p[1];
	height = p[2];
	xOffset = p[3];
	yOffset = p[4];
}

void TransformViewport(Vport)
{
}

void Scale(void *)
{
}

// Works out the corners of an isometric diamond from two opposite ones.
// Returns 0 when the points are too close together.
unsigned char figureDiamond(Point p1, Point p2, Point &c1, Point &c2, Point &c3, Point &c4)
{
	int dx = p2.f_00 - p1.f_00;
	int dy = p2.f_02 - p1.f_02;
	int rise = dy << 4;
	int run = dx;
	int slope = 0;
	int a, b;

	if (rise < 48 && rise > -48 && run < 6 && run > -6)
		return 0;
	if (run)
		slope = rise / run;
	if (slope > 8 || slope < -8 || run == 0)
	{
		if (dx & 1)
			p2.f_00--;
		if (p1.f_02 < p2.f_02)
		{
			c1 = p1;
			c3 = p2;
		}
		else
		{
			c1 = p2;
			c3 = p1;
		}
		a = c3.f_02 - (c3.f_00 >> 1);
		b = c1.f_02 + (c1.f_00 >> 1);
		c4.f_00 = b - a;
		c4.f_02 = -(c4.f_00 >> 1) + b;
		c2.f_00 = c1.f_00 + (c3.f_00 - c4.f_00);
		c2.f_02 = c1.f_02 + (c3.f_02 - c4.f_02);
	}
	else
	{
		if (!dx & 1)
			p2.f_00--;
		if (p1.f_00 < p2.f_00)
		{
			c4 = p1;
			c2 = p2;
		}
		else
		{
			c4 = p2;
			c2 = p1;
		}
		a = c2.f_02 - (c2.f_00 >> 1);
		b = c4.f_02 + (c4.f_00 >> 1);
		c1.f_00 = b - a;
		c1.f_02 = -(c1.f_00 >> 1) + b;
		c3.f_00 = c4.f_00 + (c2.f_00 - c1.f_00);
		c3.f_02 = c4.f_02 + (c2.f_02 - c1.f_02);
	}
	return 1;
}

void DrawDiamond(Vport *vport, short x1, short y1, short x2, short y2, unsigned char color)
{
	Point c1, c2, c3, c4;

	if (figureDiamond(Point(x1, y1), Point(x2, y2), c1, c2, c3, c4))
	{
		DrawLine(vport, c1.f_00, c1.f_02, c4.f_00, c4.f_02, color);
		DrawLine(vport, c4.f_00, c4.f_02, c3.f_00, c3.f_02, color);
		DrawLine(vport, c3.f_00, c3.f_02, c2.f_00, c2.f_02, color);
		DrawLine(vport, c2.f_00, c2.f_02, c1.f_00, c1.f_02, color);
	}
}

void DrawFilledDiamond(Vport *vport, short x1, short y1, short x2, short y2, unsigned char color)
{
	Point c1, c2, c3, c4;

	if (figureDiamond(Point(x1, y1), Point(x2, y2), c1, c2, c3, c4))
	{
		int y, left, right, leftStep, rightStep;

		for (y = c1.f_02, left = c1.f_00, right = c1.f_00 + 1, leftStep = -2, rightStep = 2;
		     y <= c3.f_02; y++, left += leftStep, right += rightStep)
		{
			DrawLine(vport, left, y, right, y, color);
			if (y == c4.f_02)
				leftStep = 2;
			if (y == c2.f_02)
				rightStep = -2;
		}
	}
}
