#ifndef CMPOINT_H
#define CMPOINT_H

#include "CMOUSE.H"

// Whether the mouse interrupt may draw the pointer.
enum CatState
{
	CAT_BUSY,
	CAT_FREE
};

enum MouseButton
{
	NO_BUTTON,
	LEFT_BUTTON,
	RIGHT_BUTTON,
	BOTH_BUTTONS
};

enum MouseSelectCommand
{
	NO_SELECT,
	SELECT_CLICK,
	SELECT_DOUBLE_CLICK,
	SELECT_RELEASE,
	SELECT_MOVE
};

class MousePointer : public MouseHandler
{
public:
	MousePointer(void);
	static void pushCat(CatState);
	static void popCat(void);
	static unsigned old_x;
	static unsigned old_y;
	static unsigned char visible;
	static unsigned catSP;
	static CatState cat[5];
};

#endif
