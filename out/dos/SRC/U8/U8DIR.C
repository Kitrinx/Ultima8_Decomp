// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r- -y
// name: U8DIR.C

#include <string.h>
#include "NPC.H"
#include "FEXIST.H"
#include "FILESPEC.H"
#include "U8DIR.H"
#include "SCRATCHM.H"

char StaticDir[9] = "static";
char GamedatDir[9] = "gamedat";
char SavedgamesDir[9] = "savegame";
char SoundDir[9] = "sound";
char EditorDir[9] = "editor";
char UsecodeDir[80] = "usecode";
char *shapeFileName = "u8shapes.flx";
char *typeNamesFileName = "typename.dat";
char *fixedFileDesc = "fixed.dat";
char *nonfixedFileDesc = "nonfixed.dat";
char *globFileName = "glob.flx";
char *endgameFileName = "endgame.skf";
char *transformPaletteFileName = "xformpal.dat";
char *musicFileName = "music.flx";
char *gumpsFileName = "u8gumps.flx";
char *fontsFileName = "u8fonts.flx";
char *quoteDataFile = "quotes.dat";
char *usecodeFileName = "eusecode.flx";
char *introFileName = "eintro.skf";
char *speechFileName = "espeech.flx";
char *creditDataFile = "ecredits.dat";
char *copyrightStatement = "Ultima VIII: Pagan\015\012Copyright (c) 1994 by Origin Systems.\015\012All Rights Reserved.\015\012";
char *endGameFlagName = "I81b4u83";
char *quotesFlagName = "OuPb4Ip";
char *flagExtension = "iCu";
char *thanks[15] = {"Rest, Avatar...you will need it!\015\012", "Va prendre des forces, Avatar...tu en auras besoin!\015\012", "Ruht Euch aus, Avatar...denn in Pagan \201berleben nur die Starken!\015\012", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Use the current language's copy of a file if it exists.
// The first letter of the name is the language letter, e.g. "eusecode.flx".
void languageSensitiveFile(char *dir, char *name)
{
	char path[80];

	strcpy(path, name);
	path[0] = avatar.getLanguageChar();

	if (!FileExists(fileSpec(NULL, dir, path, NULL)))
		return;

	strcpy(name, path);

}
