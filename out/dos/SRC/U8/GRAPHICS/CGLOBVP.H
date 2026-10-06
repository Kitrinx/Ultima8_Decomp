#ifndef CGLOBVP_H
#define CGLOBVP_H

class Vport;

class GlobalVport
{
public:
	void create_main_screen(void);
	static void set(Vport *v) { global_ptr = v; }
	static Vport *global_ptr;
	static Vport *main_screen;
};

#endif
