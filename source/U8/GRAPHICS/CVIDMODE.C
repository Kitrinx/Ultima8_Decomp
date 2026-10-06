// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\CVIDMODE.C

#include <dos.h>
#include "CVIDMODE.H"

// CRTC index/value pairs for the tweaked 256-colour modes.
unsigned params[17] =
{
	0x6b00, 0x5901, 0x5a02, 0x8e03, 0x5e04, 0x8a05, 0x4009, 0x0014,
	0xe317, 0x0d06, 0x3e07, 0xea10, 0xac11, 0xe715, 0x0616, 0x4109,
	0x0000
};

void VideoMode::grab(void)
{
	_AH = 0x0f;
	geninterrupt(0x10);
	_AH = 0;
	mode = _AX;
}

void VideoMode::activate(void)
{
	unsigned char crtc;
	int i;
	int wanted = mode;

	grab();
	if (mode == wanted)
		return;
	// mode = wanted
	asm {
		les bx, this
		mov ax, wanted
		mov es:[bx], ax
	}
	unsigned char biosMode = mode;

	switch (mode)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 0x13:
		asm {
			mov ah, 0
			mov al, biosMode
			int 10h
		}
		break;

	case 0x60:
		asm {
			mov ah, 0
			mov al, 13h
			int 10h
		}
		outport(0x3c4, 0x0604);
		outport(0x3c4, 0x0100);
		outportb(0x3c2, 0xc3);
		outport(0x3c4, 0x0300);
		outportb(0x3d4, 0x11);
		crtc = inportb(0x3d5);
		outportb(0x3d4, crtc & 0x7f);
		for (i = 7; i < 17; i++)
			outport(0x3d4, params[i]);
		break;

	case 0x61:
		asm {
			mov ah, 0
			mov al, 13h
			int 10h
		}
		outport(0x3c4, 0x0604);
		outportb(0x3d4, 0x11);
		crtc = inportb(0x3d5);
		outportb(0x3d5, crtc & 0x7f);
		for (i = 6; i < 9; i++)
			outport(0x3d4, params[i]);
		break;

	case 0x62:
		asm {
			mov ah, 0
			mov al, 13h
			int 10h
		}
		outport(0x3c4, 0x0604);
		outport(0x3c4, 0x0100);
		outportb(0x3c2, 0xc3);
		outport(0x3c4, 0x0300);
		outportb(0x3d4, 0x11);
		crtc = inportb(0x3d5);
		outportb(0x3d5, crtc & 0x7f);
		for (i = 6; i < 16; i++)
			outport(0x3d4, params[i]);
		outport(0x3d4, 0x4009);
		break;

	case 0x63:
		asm {
			mov ah, 0
			mov al, 13h
			int 10h
		}
		outport(0x3c4, 0x0604);
		outport(0x3c4, 0x0100);
		outportb(0x3c2, 0x67);
		outport(0x3c4, 0x0300);
		outportb(0x3d4, 0x11);
		crtc = inportb(0x3d5);
		outportb(0x3d5, crtc & 0x7f);
		for (i = 0; i < 9; i++)
			outport(0x3d5, params[i]);
		outport(0x3d4, 0x2d13);
		break;

	case 0x64:
		asm {
			mov ah, 0
			mov al, 13h
			int 10h
		}
		outport(0x3c4, 0x0604);
		outport(0x3c4, 0x0100);
		outportb(0x3c2, 0xe7);
		outport(0x3c4, 0x0300);
		outportb(0x3d4, 0x11);
		crtc = inportb(0x3d5);
		outportb(0x3d5, crtc & 0x7f);
		for (i = 0; i < 16; i++)
			outport(0x3d5, params[i]);
		outport(0x3d4, 0x2d13);
		break;

	case 0x7f:
		asm {
			mov al, 3
			mov ah, 0
			int 10h
			mov ax, 0500h
			int 10h
		}
		break;
	}
}
