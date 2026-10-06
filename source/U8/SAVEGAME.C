// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: SAVEGAME.C

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "CEXIT.H"
#include "SCRATCHM.H"
#include "FILESPEC.H"
#include "FEXIST.H"
#include "SAVEGAME.H"

inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }

char *gameFileListId = "Ultima 8 SaveGame File.";

SavedGames::SavedGames(void)
{
	strcpy(name, fileSpec(0, SavedgamesDir, "u8save", 0));
	number = -1;
	isNew = 1;
}

void SavedGames::setNum(int num)
{
	if (num > 999)
		num = 999;
	else if (num < 0)
		num = 0;
	if (number == num)
		return;
	if (file.is_valid())
		file.close();
	char filename[80];
	sprintf(filename, "%s.%03d", name, num);
	if (FileExists(filename))
	{
		file.open(filename, ReadWrite);
		isNew = 0;
		return;
	}
	file.forceOpen(filename, ReadWrite);
	isNew = 1;
}

// Unpacks the saved game into the gamedat folder.
void SavedGames::unpack(int num)
{
	char *id;
	long size;
	char *filename;
	int count;
	int i;

	setNum(num);
	if (isNew)
		return;
	id = new char[strlen(gameFileListId) + 1];
	file.read(id, strlen(gameFileListId) + 1);
	if (strcmp(gameFileListId, id) != 0)
		halt(__FILE__, 74);	// __LINE__
	delete id;
	BaseFile temp;
	size = 0;
	FileSpec spec(0, GamedatDir, "*", "*");
	spec.unlinkAll();
	count = 0;
	file.read((char *)&count, sizeof(count));
	for (i = 0; i < count; i++)
	{
		file.read((char *)&size, sizeof(size));
		filename = new char[size];
		file.read(filename, size);
		unlink(fileSpec(0, GamedatDir, filename, 0));
		temp.forceOpen(fileSpec(0, GamedatDir, filename, 0), ReadWrite);
		file.read((char *)&size, sizeof(size));
		temp.relocate(file.tell(), 0, size, &file);
		temp.close();
		delete filename;
	}
}

// Packs every file in a folder into the saved game.
void SavedGames::pack(int num, char *dir)
{
	long size;
	char *filename;
	int count;
	char path[80];
	char saveName[80];

	setNum(num);
	if (dir == 0)
		dir = GamedatDir;
	strcpy(path, dir);
	if (!isNew)
	{
		file.close();
		sprintf(saveName, "%s.%03d", name, num);
		unlink(saveName);
		file.forceOpen(saveName, ReadWrite);
	}
	file.write(gameFileListId, strlen(gameFileListId) + 1);
	BaseFile temp;
	size = 0;
	FileSpec spec(0, dir, "*", "DAT");
	spec.getFirst();
	count = 0;
	file.write((char *)&count, sizeof(count));
	do
	{
		count++;
		filename = spec;
		temp.open(filename, ReadOnly);
		filename = fileSpec(0, 0, spec.name, spec.ext);
		size = strlen(filename) + 1;
		file.seek(0, FromEnd);
		file.write((char *)&size, sizeof(size));
		file.write(filename, size);
		size = temp.size();
		file.write((char *)&size, sizeof(size));
		file.relocate(0, file.tell(), size, &temp);
		temp.close();
	} while (spec.getNext());
	file.seek(strlen(gameFileListId) + 1);
	file.write((char *)&count, sizeof(count));
}
