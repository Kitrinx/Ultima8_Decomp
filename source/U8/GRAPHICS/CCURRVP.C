// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\CCURRVP.C

#include "CCURRVP.H"
#include "CVPORT.H"

Vport *GlobalVport::global_ptr = 0;

CurrentVport::CurrentVport(Vport *v)
{
	saved = global_ptr;
	set(v);
}

CurrentVport::~CurrentVport(void)
{
	set(saved);
}
