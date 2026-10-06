#ifndef CRECT_H
#define CRECT_H

class Point;

class Point
{
public:
	int f_00;
	int f_02;

	Point(void);
	Point(short, short);
	void set(short, short);
};

class Rect : public Point
{
public:
	int f_04;
	int f_06;

	Rect(void);
	Rect(short, short, short, short);
	void moveto(short, short);
	unsigned char moverel(short, short);
	int contains(Point);
	unsigned char contains(short, short);
	void clip(Rect &);
	unsigned char intersects(Rect &);
	int width(void);
	void set(short, short, short, short);
	int height(void);
};

#endif
