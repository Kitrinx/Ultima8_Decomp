#ifndef CPALETTE_H
#define CPALETTE_H

class MemHandle;
class RGB;

// A run of `count` palette entries starting at colour index `start`.
class PaletteData
{
public:
	int count;
	unsigned char start;
	RGB *colors;

	PaletteData(void) { count = 0; colors = 0; }
	PaletteData(unsigned char first, short n) { init(first, n); }
	PaletteData(unsigned char first, short n, RGB *rgb) { init(first, n, rgb); }
	~PaletteData(void);
	void init(unsigned char, short);
	void init(RGB *);
	void init(MemHandle &);
	void init(unsigned char, short, RGB *);
	void grab(void);
	void activate(unsigned char, short, int);
	void activate(int steps) { activate(start, count, steps); }
	void grab(unsigned char, short);
	void clear(RGB &, unsigned char, short);
	void copy_from(PaletteData &, unsigned char, short);
	void copy_from(PaletteData &p) { copy_from(p, p.start, p.count); }
	void set(RGB &, unsigned char);
	unsigned char fade_step(PaletteData &);
};

class Palette : public PaletteData
{
public:
	void init(MemHandle &);
	void fade_to_color(RGB &, int, unsigned char, short);
	void fade_from_color(RGB &, int, unsigned char, short);
};

extern "C" void pascal GETCOLORBANKRGB(unsigned char, short, RGB *);
extern "C" void pascal SETCOLORBANKRGB(unsigned char, short, RGB *);
extern "C" void pascal SETCOLORRGB(unsigned char, RGB *);

#endif
