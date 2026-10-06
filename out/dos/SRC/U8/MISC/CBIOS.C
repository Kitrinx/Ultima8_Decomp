// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MISC\CBIOS.C

#include <dos.h>
#include "CBIOS.H"

void conputs(const char *s)
{
	char c;
	union REGS in, out;

	in.h.ah = 0x0e;
	in.h.bl = 0x0f;
	in.h.bh = 0;
	while ((c = *s++) != 0)
	{
		for (;;)
		{
			in.h.al = c;
			int86(0x10, &in, &out);
			if (c != '\n')
				break;
			c = '\r';
		}
	}
}

int congetkey(void)
{
	union REGS regs;
	int key;

	regs.h.ah = 0;
	int86(0x16, &regs, &regs);
	key = regs.h.al;
	if (key == 0)
		key = regs.h.ah | 0x100;
	return key;
}

void congetcursor(CursorPosition &pos, char page)
{
	union REGS in, out;

	in.h.ah = 3;
	in.h.bh = page;
	int86(0x10, &in, &out);
	pos.row = out.h.dh;
	pos.column = out.h.dl;
}

void consetcursor(CursorPosition &pos, char page)
{
	union REGS in, out;

	in.h.ah = 2;
	in.h.bh = page;
	in.h.dh = pos.row;
	in.h.dl = pos.column;
	int86(0x10, &in, &out);
}

// Finds the text mode's height by trying to park the cursor on rows 49 and 39.
ScanCodes congetscans(void)
{
	CursorPosition saved, test;
	ScanCodes scans = SCANS_400;

	congetcursor(saved, 0);
	test.column = 0;
	test.row = 49;
	consetcursor(test, 0);
	congetcursor(test, 0);
	if (test.row != 49)
	{
		scans = SCANS_350;
		test.row = 39;
		consetcursor(test, 0);
		congetcursor(test, 0);
		if (test.row != 49)
			scans = SCANS_200;
	}
	consetcursor(saved, 0);
	return scans;
}

void consetscans(ScanCodes scans)
{
	union REGS in, out;

	in.h.ah = 0x12;
	in.h.bl = 0x30;
	in.h.al = scans;
	int86(0x10, &in, &out);
}

void dosputs(const char *s)
{
	char c;
	union REGS in, out;

	in.h.ah = 2;
	while ((c = *s++) != 0)
	{
		for (;;)
		{
			in.h.dl = c;
			int86(0x21, &in, &out);
			if (c != '\n')
				break;
			c = '\r';
		}
	}
}
