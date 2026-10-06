// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\INITSYS.C

#include <dos.h>
#include <phapi.h>
#include "CEXIT.H"
#include "CICRTC.H"
#include "CVPORT.H"
#include "CDESC.H"

unsigned short TextSelector;

void GetBiosSegment(void)
{
	if (DosGetBIOSSeg((SEL *)&BiosSegment))
		Fatal("Failed to get Bios Segment Descriptor!");
}

void SetVgaScreenDescriptor(void)
{
	if ((VgaScreenDescriptor = MapSegment(0xA000, 0xFA00)) == 0)
		Fatal("Failed to get Screen Descriptor.");
	if (DosMapRealSeg(0xB800, 8000, &TextSelector))
		Fatal("Failed to get Screen Descriptor.");
}

unsigned short MapSegment(unsigned short segment, unsigned short size)
{
	unsigned short sel;

	return DosMapRealSeg(segment, size, &sel) ? 0 : sel;
}
