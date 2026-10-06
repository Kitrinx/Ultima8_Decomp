// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ARMORDAT.C

#include "STDINT.H"

// Armor class bonus by frame of each armor type; -128 ends a table.
char ArmorData::shieldBonus[3] = {4, 3, -128};
char ArmorData::leggingBonus[3] = {2, 3, -128};
char ArmorData::helmetBonus[6] = {4, 2, 2, 3, 3, -128};
char ArmorData::armourBonus[4] = {4, 3, 2, -128};
char ArmorData::armGuardsBonus[5] = {2, 2, 1, 3, -128};
char ArmorData::magArmorBonus[2] = {8, -128};
char ArmorData::magArmorTwoBonus[2] = {6, -128};
char ArmorData::magShieldBonus[2] = {5, -128};
char ArmorData::magArmsBonus[2] = {4, -128};
char ArmorData::magLegsBonus[2] = {4, -128};
char ArmorData::magHelmBonus[2] = {5, -128};
char ArmorData::clothesBonus[14] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -128};
