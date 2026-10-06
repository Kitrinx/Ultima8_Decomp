// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\CICRTC.C

#include <dos.h>
#include "CICRTC.H"

unsigned CrtcStatusPort;
unsigned CrtcAddressRegister;
unsigned CrtcDataRegister;
unsigned BiosSegment;

void InitializeCrtc(void)
{
	unsigned far *crtcPort = (unsigned far *)MK_FP(BiosSegment, 0x63);
	CrtcAddressRegister = *crtcPort;
	CrtcDataRegister = CrtcAddressRegister + 1;
	CrtcStatusPort = CrtcDataRegister + 5;
}
