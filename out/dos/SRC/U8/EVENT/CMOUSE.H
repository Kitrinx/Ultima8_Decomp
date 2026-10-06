#ifndef CMOUSE_H
#define CMOUSE_H

extern "C" void far mouseHook(void);
extern int fakeStackOverflowFlag;

class MouseDevice
{
public:
	MouseDevice(void);
	~MouseDevice(void);
	static void init(void);
	static int reset(void);
	static void install_mouse_hook(void);
	static void remove_mouse_hook(void);
	static void move_to(int, int);
	static void set_x_bounds(int, int);
	static void set_y_bounds(int, int);
	static void set_speed(int, int);
	static int get_last_x(void);
	static int get_last_y(void);
	static void freeze(void);
	static void release(void);
	static char is_installed(void);
	static int get_number_of_buttons(void);
	static void set_bounds(int, int, int, int);
	static unsigned original_handler_mask;
	static unsigned original_handler[2];
	static unsigned char installed_flag;
	static unsigned number_of_buttons;
	static unsigned mouseLastX;
	static unsigned mouseLastY;
};

class MouseHandler
{
public:
	unsigned slot;
	unsigned mask;

	MouseHandler(unsigned);
	~MouseHandler(void);
	virtual void handle(int, int, int, int) = 0;
	void suspend(void);
	void resume(void);
	static MouseHandler *handlers[4];
};

#endif
