// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\TYPE.C

#include <string.h>
#include <mem.h>
#include "CACHE.H"
#include "ITEM.H"
#include "ITEMCACH.H"
#include "GLOB.H"
#include "SHAPHAND.H"
#include "CFILE.H"
#include "FEXIST.H"
#include "FILESPEC.H"
#include "CEXIT.H"
#include "SCRATCHM.H"
#include "STATREST.H"
#include "UPROCESS.H"
#include "TYPE.H"

// Inline in the shared headers when this file was built.
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }

char *typeFlagFileName = "typeflag.dat";
TypeObject GlobalTypes;

// A shape with no entry: solid, one footpad square, no height.
void TypeFlag::init(void)
{
	family = GENERIC_FAMILY;
	xSize = 1;
	ySize = 1;
	zSize = 0;
	fixed = FALSE;
	solid = TRUE;
	sea = FALSE;
	land = FALSE;
	damaging = FALSE;
	noisy = FALSE;
	draw = FALSE;
	ignore = FALSE;
	occl = FALSE;
	bag = FALSE;
	roof = FALSE;
	trans = FALSE;
	editor = FALSE;
	explode = FALSE;
	unknown1 = FALSE;
	unknown2 = FALSE;
	animType = 0;
	animData = 0;
	animSpeed = 0;
	weight = 0;
	volume = 0;
	equipType = 0;
}

TypeObject::TypeObject(void)
{
	if ((typeFlags = new TypeFlag[NUM_TYPES]) == 0)
		outOfMemory(__FILE__, 107);	// __LINE__ in the original file
	loadTypes();
	loadTypeNames();
}

void TypeObject::loadTypes(void)
{
	char name[80];
	strcpy(name, fileSpec(0, StaticDir, typeFlagFileName, 0));
	BaseFile file(name, (OpenMode)2);
	file.read((char *)typeFlags, (long)NUM_TYPES * sizeof(TypeFlag), 0);
}

TypeFlag TypeObject::getTypeFlag(unsigned short type)
{
	return typeFlags[type];
}

char *typeNameFile = "typename.dat";

// Type 0 has no name, so the file is read from its second entry.
void TypeObject::loadTypeNames(void)
{
	char name[80];
	strcpy(name, fileSpec(0, EditorDir, typeNameFile, 0));
	typeNames = 0;
	if (!FileExists(name))
		return;
	if ((typeNames = new char[(NUM_TYPES - 1) * TYPE_NAME_LEN]) == 0)
		outOfMemory(__FILE__, 166);	// __LINE__ in the original file
	BaseFile file(name, (OpenMode)2);
	file.read(typeNames, (long)(NUM_TYPES - 1) * TYPE_NAME_LEN, TYPE_NAME_LEN);
}

char *TypeObject::getTypeName(short type)
{
	return type ? typeNames + (type - 1) * TYPE_NAME_LEN : 0;
}

int getFamilyOfType(unsigned short type)
{
	return GlobalTypes.typeFlags[type].family;
}
