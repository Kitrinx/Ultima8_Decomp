#ifndef CAMERA_H
#define CAMERA_H

#include "..\GLIB\PROCESS.H"
#include "BASECAM.H"
#include "DLIST.H"

class BaseFile;
class FastItem;
class Item;
class Vport;
class Zbuffer;

// The game view. _h and _v are the screen position of the view's corner,
// _x, _y, _z the world point it is centred on.
class Camera : public BaseCamera
{
public:
	Camera(void);
	virtual void show(void);
	virtual void show(short, short, short, short, unsigned char);
	virtual void slam(void);
	virtual void initPalette(void);
	static void add_items_to_list(void);
	static void renumber_shadow_squares(short, short);
	static void move_to(unsigned short, unsigned short, unsigned char, short);
	static int scrollTo(unsigned short, unsigned short, unsigned char, int);
	static void drawBounds(Vport *);
	static void position(void);
	static void startQuake(short);
	static void stopQuake(void);
	static void slamQuake(void);
	static void invertScreen(unsigned char);
	static void setScreenInverted(unsigned char);
	static void setCenterOn(unsigned short);
	static void move_rel(int, int, int);
	static void set_roof(short);
	static void find_new_roof(Item);
	static unsigned char roof_item_moved(void);
	static void init(void);
	static unsigned getX(void);
	static unsigned getY(void);
	static unsigned char getZ(void);
	static int roof(void);
	static int _h;
	static int _v;
	static unsigned char _moved;
	static unsigned char _drawn;
	static unsigned char lastShowFootpads;
	static int _roof;
	static unsigned roof_item_x;
	static unsigned roof_item_y;
	static unsigned char roof_item_z;
	static unsigned short centeredOn;
	static unsigned char needAnim;
	static unsigned globBounds;
	static unsigned globBoundsX;
	static unsigned globBoundsY;
	static char quakeMagnitude;
	static char quakeX;
	static char quakeY;
	static char quakeDX;
	static char quakeDY;
	static unsigned char displayListCount;
	static unsigned char disableDrawBit;
	static unsigned char drawUgliKolur;
	static unsigned char screenInverted;
	static unsigned char skipFrames;
	static ScreenArea _validarea;	// part of the screen still showing the right picture
	static unsigned _x;
	static unsigned _y;
	static unsigned char _z;
	static unsigned char catchingAvatar;
};

class CameraProcess : public Process
{
public:
	CameraProcess(void);
	virtual void process(void);
	virtual void fail(long);
	virtual void load(BaseFile *);
	virtual void save(BaseFile *);
	virtual void postLoad(void);
};

// Scrolls the camera to a world point, speeding up and slowing down.
class ScrollProcessPlusPlus : public Process
{
public:
	int x;
	int y;
	int z;
	int destX;
	int destY;
	int destZ;
	int step;
	int steps;
	int errX;
	int errY;
	int errZ;
	int dx;
	int dy;
	int dz;
	int sx;
	int sy;
	int sz;
	int maxSpeed;
	int speed;

	ScrollProcessPlusPlus(unsigned short, unsigned short, unsigned char, int);
	virtual void process(void);
};

// Scrolls the screen in a line to a world point, Bresenham style.
class ScrollProcess : public Process
{
public:
	int dx;
	int dy;
	int incStraight;
	int incDiagonal;
	int error;
	short destX;
	short destY;
	short x;
	short y;
	int speed;
	unsigned char octant;

	ScrollProcess(unsigned short, unsigned short, unsigned char, int);
	virtual void process(void);
};

void initU8Camera(void);
void ScreenToWorldCoords(short, short, unsigned short &, unsigned short &);
void ScreenToWorldCoords(short, short, unsigned short &, unsigned short &, unsigned char &, FastItem *, unsigned char *);
void WorldToScreenCoords(unsigned short, unsigned short, short &, short &);
void SwapInt(int *, int *);
void Line(Vport *, int, int, int, int, unsigned char);

extern unsigned char hideEditorItems;
extern Camera *theCamera;
extern CameraProcess *theCameraProcess;
extern Zbuffer theZbuffer;

#endif
