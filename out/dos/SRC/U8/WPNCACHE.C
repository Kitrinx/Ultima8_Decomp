// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: WPNCACHE.C

#include "NPC.H"
#include "ERROR.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "ANIMCACH.H"
#include "ASTREAM.H"
#include "WPNCACHE.H"

// Inline in the shared headers when this file was built.
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline SharedFile::SharedFile(void) {}
inline FlexFile::FlexFile(void) {}

char *weaponOverlayFile = "wpnovlay.dat";
char *weaponString[15] = {"Mace      ", "Sword     ", "Axe       ", "Dagger    ", "Earth Glow", "Air Glow  ", "Fire Glow ", "", "", "", "", "", "", "", ""};
unsigned WeaponCache::weaponType = 0;
unsigned WeaponCache::overlayIndex = 0;
unsigned WeaponCache::typeArray[15] = {241, 243, 244, 245, 0x18e, 0x18f, 0x190, 0, 0, 0, 0, 0, 0, 0, 0};
WeaponInfo WeaponCache::typeIndirectionArray[15] =
{
	{0x21c, 0, 9, 4, 0, 0, 0, 4},
	{0x21d, 1, 12, 5, 0, 0, 0, 2},
	{0x21e, 1, 12, 5, 0, 0, 0, 2},
	{0x1a4, 1, 13, 5, 0, 0, 0, 2},
	{0x1a3, 2, 14, 6, 0, 0, 0, 6},
	{0x1a2, 3, 7, 3, 0, 0, 0, 2},
	{0x1a1, 0, 11, 4, 0, 0, 0, 4},
	{0x32f, 0, 9, 11, 0, 0, 0, 36},
	{0x336, 2, 14, 8, 2, 2, 2, 38},
	{0x330, 0, 11, 4, 0, 0, 0, 100},
	{0x335, 1, 12, 5, 1, 0, 0, 50},
	{0x334, 3, 7, 7, 4, 0, 1, 34},
	{0x333, 1, 13, 5, 7, 0, 0, 34},
	{0x332, 1, 13, 5, 0, 4, 5, 34},
	{0x331, 1, 12, 5, 1, 0, 0, 42}
};

void WeaponCache::load(unsigned short type)
{
	int anim;
	AnimStreamHeader header;

	weaponType = type;
	unload();
	file.init(fileSpec(0, StaticDir, weaponOverlayFile, 0), ReadOnly, -1);
	overlayIndex = getWeaponOverlayIndex(type);
	for (anim = 0; anim < 64; anim++)
	{
		header = AnimCache::getAnimStreamHeader(1, (AnimSet)anim);
		int frames;
		if ((frames = header.frames) != 0 && header.attack)
		{
			int size = frames * 8 * sizeof(WeaponOffset);
			wpnAnimTable[anim] = (WeaponOffset *)new char[size];
			if (!wpnAnimTable[anim])
				halt(__FILE__, 160);	// __LINE__
			file.readRecord(anim, wpnAnimTable[anim], size, size * overlayIndex);
		}
	}
	file.close();
}

void WeaponCache::unload(void)
{
	int anim;

	for (anim = 0; anim < 64; anim++)
	{
		if (wpnAnimTable[anim])
		{
			delete wpnAnimTable[anim];
			wpnAnimTable[anim] = 0;
		}
	}
}

int WeaponCache::getWeaponIndex(unsigned short type)
{
	int i;

	for (i = 0; i < 15; i++)
		if (typeIndirectionArray[i].type == type)
			return i;
	halt(__FILE__, 194, "%d.\n", type);	// __LINE__
}

int WeaponCache::getWeaponOverlayIndex(unsigned short type)
{
	return typeIndirectionArray[getWeaponIndex(type)].overlay;
}

void WeaponCache::setWeaponType(unsigned short type)
{
	if (weaponType != type)
		load(type);
}

void WeaponCache::setOverlayIndex(unsigned short overlay)
{
	if (overlayIndex != overlay)
	{
		int i;

		for (i = 0; i < 15; i++)
		{
			if (typeIndirectionArray[i].overlay == overlay)
			{
				setWeaponType(typeIndirectionArray[i].type);
				return;
			}
		}
		halt(__FILE__, 232);	// __LINE__
	}
}

void WeaponCache::getOffsets(unsigned short type, AnimSet anim, char dir, unsigned char animFrame,
	unsigned short &shape, char &x, char &y, unsigned short &frame)
{
	AnimStreamHeader header;
	WeaponOffset offset;

	if (weaponType != type)
		load(type);
	if (!wpnAnimTable[anim])
		halt(__FILE__, 245);	// __LINE__
	header = AnimCache::getAnimStreamHeader(1, anim);
	offset = wpnAnimTable[anim][dir * header.frames + animFrame];
	shape = typeArray[overlayIndex];
	x = offset.x;
	y = offset.y;
	frame = offset.frame;
}

unsigned char WeaponCache::isWeaponDrawn(unsigned short type, AnimSet anim)
{
	if (weaponType != type)
		load(type);
	return wpnAnimTable[anim] != 0;
}

int WeaponCache::getDamageModifier(unsigned short type)
{
	return typeIndirectionArray[getWeaponIndex(type)].damageModifier;
}

int WeaponCache::getBaseDamage(unsigned short type)
{
	return typeIndirectionArray[getWeaponIndex(type)].baseDamage;
}

int WeaponCache::getDexAttackBonus(unsigned short type)
{
	return typeIndirectionArray[getWeaponIndex(type)].dexAttackBonus;
}

int WeaponCache::getDexDefendBonus(unsigned short type)
{
	return typeIndirectionArray[getWeaponIndex(type)].dexDefendBonus;
}

int WeaponCache::getArmorClassBonus(unsigned short type)
{
	return typeIndirectionArray[getWeaponIndex(type)].armorClassBonus;
}

int WeaponCache::getDamageType(unsigned short type)
{
	return typeIndirectionArray[getWeaponIndex(type)].damageType;
}

void WeaponCache::flush(void)
{
	unload();
	weaponType = 0;
}

WeaponOffset *WeaponCache::wpnAnimTable[64];
FlexFile WeaponCache::file;
