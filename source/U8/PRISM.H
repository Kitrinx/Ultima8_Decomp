#ifndef PRISM_H
#define PRISM_H

// A box in world space from corner (x, y, z) to corner (x2, y2, z2).
class Prism
{
public:
	int x;
	int y;
	unsigned char z;
	int x2;
	int y2;
	unsigned char z2;

	void set(short, short, unsigned char, short, short, unsigned char);
	unsigned char intersects(Prism *);
	void moveRel(short, short, short);
};

#endif
