#ifndef TYPE_H
#define TYPE_H

class TypeFlag;

#define NUM_TYPES		2048
#define NUM_FRAMES		256
#define TYPE_NAME_LEN	32

// One bit per frame of a shape.
class FrameSet
{
public:
	unsigned char bits[NUM_FRAMES / 8];

	FrameSet(void) { init(); }
	void init(void);
	void add(unsigned short);
	char contains(unsigned short);
};

// Marks every shape and frame placed on the map, so they can be loaded in one pass.
class ShapePreloader
{
public:
	static char init(void);
	static char uninit(void);
	static FrameSet *getFrameSet(unsigned short);
	static void add(unsigned short, unsigned short);
	static char contains(unsigned short, unsigned short);
	static char make(void);
	static void preload(unsigned char);
	static FrameSet *type[2];
};

// Shape families, from the high nibble of a type's second flag byte.
#define GENERIC_FAMILY		0
#define QUAL_FAMILY			1
#define QUAN_FAMILY			2
#define GLOBEGG_FAMILY		3
#define UNKEGG_FAMILY		4
#define BREAKABLE_FAMILY	5
#define CONTAINER_FAMILY	6
#define MONSTEREGG_FAMILY	7
#define TELEPORTEGG_FAMILY	8
#define REAGENT_FAMILY		9

// One record of typeflag.dat.
class TypeFlag
{
public:
	unsigned fixed : 1;
	unsigned solid : 1;
	unsigned sea : 1;
	unsigned land : 1;
	unsigned occl : 1;
	unsigned bag : 1;
	unsigned damaging : 1;
	unsigned noisy : 1;
	unsigned draw : 1;
	unsigned ignore : 1;
	unsigned roof : 1;
	unsigned trans : 1;
	unsigned family : 4;
	unsigned equipType : 4;
	unsigned xSize : 4;
	unsigned ySize : 4;
	unsigned zSize : 4;
	unsigned animType : 4;
	unsigned animData : 4;
	unsigned animSpeed : 4;
	unsigned editor : 1;
	unsigned explode : 1;
	unsigned unknown1 : 1;
	unsigned unknown2 : 1;
	unsigned char weight;
	unsigned char volume;

	void init(void);

	// Footpad size in world units; a non-fixed shape is never flat.
	void getWorldSize(int &x, int &y, int &z)
	{
		x = xSize * 32;
		y = ySize * 32;
		z = zSize * 8;
		z == 0 && !fixed ? z = 1 : 0;
	}

	// The same in display list units, a quarter of world x and y.
	void getFootpad(int &x, int &y, int &z)
	{
		x = xSize * 8;
		y = ySize * 8;
		z = zSize * 8;
		z == 0 && !fixed ? z = 1 : 0;
	}
};

class TypeObject
{
public:
	char *typeNames;
	TypeFlag *typeFlags;

	TypeObject(void);
	void loadTypes(void);
	TypeFlag getTypeFlag(unsigned short);
	void loadTypeNames(void);
	char *getTypeName(short);
};

class WorldPoint
{
public:
	unsigned x;
	unsigned y;
	unsigned char z;

	WorldPoint(void) {}
	WorldPoint(unsigned px, unsigned py, unsigned char pz) { x = px; y = py; z = pz; }
	void set(unsigned px, unsigned py, unsigned char pz) { x = px; y = py; z = pz; }
	unsigned char operator==(WorldPoint &);
};

int getFamilyOfType(unsigned short);
int guardianBark(int);

extern char *typeFlagFileName;
extern char *typeNameFile;
extern TypeObject GlobalTypes;

#endif
