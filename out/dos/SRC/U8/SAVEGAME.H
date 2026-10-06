#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "CFILE.H"

// One saved game: every file in a folder packed into "u8save.NNN".
class SavedGames
{
public:
	char name[80];
	int number;
	BaseFile file;
	unsigned char isNew;

	SavedGames(void);
	void setNum(int);
	void unpack(int);
	void pack(int, char *);
};

extern char *gameFileListId;

#endif
