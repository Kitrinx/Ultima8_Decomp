#ifndef WPNCACHE_H
#define WPNCACHE_H

#include "..\GLIB\PROCESS.H"
#include "ANIMCACH.H"
#include "..\POLIB\INC\FLEX.H"
#include "SPRITE.H"

// One weapon type: its overlay in wpnovlay.dat and its combat numbers.
struct WeaponInfo
{
	unsigned type;
	unsigned char overlay;
	unsigned char damageModifier;
	unsigned char baseDamage;
	char dexAttackBonus;
	char dexDefendBonus;
	char armorClassBonus;
	unsigned damageType;
};

// Where the weapon overlay sits in one Avatar animation frame.
struct WeaponOffset
{
	char x;
	char y;
	unsigned frame;
};

// The overlay offsets of the current weapon for each of the Avatar's animations.
class WeaponCache
{
public:
	static void load(unsigned short);
	static void unload(void);
	static int getWeaponIndex(unsigned short);
	static int getWeaponOverlayIndex(unsigned short);
	static void setWeaponType(unsigned short);
	static void setOverlayIndex(unsigned short);
	static void getOffsets(unsigned short, AnimSet, char, unsigned char, unsigned short &, char &, char &, unsigned short &);
	static unsigned char isWeaponDrawn(unsigned short, AnimSet);
	static int getDamageModifier(unsigned short);
	static int getBaseDamage(unsigned short);
	static int getDexAttackBonus(unsigned short);
	static int getDexDefendBonus(unsigned short);
	static int getArmorClassBonus(unsigned short);
	static int getDamageType(unsigned short);
	static void flush(void);
	static unsigned weaponType;
	static unsigned overlayIndex;
	static unsigned typeArray[15];
	static WeaponInfo typeIndirectionArray[15];
	static WeaponOffset *wpnAnimTable[64];
	static FlexFile file;
};

void initAnimCache(void);
void uninitAnimCache(void);

extern char *weaponOverlayFile;
extern char *weaponString[15];

#endif
