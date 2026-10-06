// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\CGLOBVP.C

#include "CGLOBVP.H"
#include "CVPORT.H"

inline Point::Point(void) {}
inline Rect::Rect(void) {}
inline GraphicYtable::GraphicYtable(void) { f_04 = 2; index = 0; }
inline Vport::Vport(void) { seg = 0; f_0f = 2; f_10 = 0; }

Vport *GlobalVport::main_screen = 0;

void GlobalVport::create_main_screen(void)
{
	if (!main_screen)
	{
		main_screen = new Vport;
		main_screen->map_to_screen(0);
		global_ptr = main_screen;
	}
}
