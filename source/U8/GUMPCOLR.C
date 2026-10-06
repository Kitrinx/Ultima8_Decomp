// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GUMPCOLR.C

// Colour remap for gump shapes; GumpColorMap selects the active table.
unsigned char defaultGumpColorMap[32] = {0, 131, 25, 22, 100, 19, 53, 255, 15, 69, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char *GumpColorMap = defaultGumpColorMap;
