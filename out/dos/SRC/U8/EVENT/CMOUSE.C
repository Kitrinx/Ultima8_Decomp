// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: EVENT\CMOUSE.C

#include <dos.h>
#include "..\MISC\CEXIT.H"
#include "CMOUSE.H"

unsigned MouseDevice::original_handler_mask = 0;
unsigned MouseDevice::original_handler[2] = {0, 0};
unsigned char MouseDevice::installed_flag;
unsigned MouseDevice::number_of_buttons;
unsigned MouseDevice::mouseLastX;
unsigned MouseDevice::mouseLastY;
MouseHandler *MouseHandler::handlers[4];

MouseDevice::MouseDevice(void)
{
	init();
}

MouseDevice::~MouseDevice(void)
{
	remove_mouse_hook();
}

void MouseDevice::init(void)
{
	_AX = 0;
	geninterrupt(0x33);
	if (_AX == 0xFFFF)
		installed_flag = 1;
	else
		installed_flag = 0;
	number_of_buttons = _BX;
	move_to(0, 0);
	mouseLastX = mouseLastY = 0;
	// Mickeys per 8 pixels.
	_AX = 0x0F;
	_CX = 3;
	_DX = 0x10;
	geninterrupt(0x33);
	install_mouse_hook();
}

int MouseDevice::reset(void)
{
	_AX = 0;
	geninterrupt(0x33);
	return _AX;
}

// Swaps mouseHook in for every driver event, keeping the old handler.
void MouseDevice::install_mouse_hook(void)
{
	_ES = FP_SEG(mouseHook);
	_DX = FP_OFF(mouseHook);
	_AX = 0x14;
	_CX = 0x3F;
	geninterrupt(0x33);
	original_handler_mask = _CX;
	original_handler[1] = _ES;
	original_handler[0] = _DX;
}

void MouseDevice::remove_mouse_hook(void)
{
	_DX = original_handler[0];
	_ES = original_handler[1];
	_AX = 0x14;
	_CX = original_handler_mask;
	geninterrupt(0x33);
}

// These four set only AL, so AH must already be 0.
void MouseDevice::move_to(int x, int y)
{
	_AL = 4;
	_CX = x;
	_DX = y;
	geninterrupt(0x33);
}

void MouseDevice::set_x_bounds(int min, int max)
{
	_AL = 7;
	_CX = min;
	_DX = max;
	geninterrupt(0x33);
}

void MouseDevice::set_y_bounds(int min, int max)
{
	_AL = 8;
	_CX = min;
	_DX = max;
	geninterrupt(0x33);
}

void MouseDevice::set_speed(int x, int y)
{
	_AL = 0x0F;
	_CX = x;
	_DX = y;
	geninterrupt(0x33);
}

int MouseDevice::get_last_x(void)
{
	if (fakeStackOverflowFlag)
		Fatal("mouse stack overflow");
	return mouseLastX;
}

int MouseDevice::get_last_y(void)
{
	if (fakeStackOverflowFlag)
		Fatal("mouse stack overflow");
	return mouseLastY;
}

// Pins the pointer where it is by shrinking both ranges to its position.
void MouseDevice::freeze(void)
{
	int x, y;

	asm {
		mov ax, 3
		int 33h
		mov x, cx
		mov y, dx
		mov ax, 7
		mov cx, x
		mov dx, x
		int 33h
		mov ax, 8
		mov cx, y
		mov dx, y
		int 33h
	}
}

void MouseDevice::release(void)
{
	asm {
		mov ax, 7
		mov cx, 0
		mov dx, 639
		int 33h
		mov ax, 8
		mov cx, 0
		mov dx, 199
		int 33h
	}
}

char MouseDevice::is_installed(void)
{
	return installed_flag;
}

int MouseDevice::get_number_of_buttons(void)
{
	return number_of_buttons;
}

void MouseDevice::set_bounds(int left, int top, int right, int bottom)
{
	set_x_bounds(left, right);
	set_y_bounds(top, bottom);
}

MouseHandler::MouseHandler(unsigned m)
{
	for (int i = 0; i < 4; i++) {
		if (handlers[i] == 0) {
			mask = m;
			slot = i;
			handlers[i] = this;
			return;
		}
	}
	Fatal("Too many handlers");
}

MouseHandler::~MouseHandler(void)
{
	handlers[slot] = 0;
}

void MouseHandler::suspend(void)
{
	handlers[slot] = 0;
}

void MouseHandler::resume(void)
{
	handlers[slot] = this;
}
