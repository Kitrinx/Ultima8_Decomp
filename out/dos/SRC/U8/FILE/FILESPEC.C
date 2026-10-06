// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: FILE\FILESPEC.C

#include <stddef.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "CFILE.H"
#include "CEXIT.H"
#include "FILESPEC.H"

#define NUM_SPECS	5

// fileSpec() results rotate through these, so a few can be used at once.
static char *specs[NUM_SPECS];
static int currentSpec = -1;

// Joins the given parts into a path, adding backslashes and the dot where
// needed. Any part may be NULL.
char *fileSpec(char *drive, char *dir, char *name, char *ext)
{
	if (currentSpec == -1)
		for (int i = 0; i < NUM_SPECS; i++)
		{
			specs[i] = new char[80];
			if (specs[i] == NULL)
				outOfMemory(__FILE__, 128);	// __LINE__
		}
	currentSpec++;
	if (currentSpec == NUM_SPECS)
		currentSpec = 0;
	specs[currentSpec][0] = 0;

	int part = 0;
	while (part != 4)
		switch (part)
		{
		case 0:
			if (drive && *drive)
				strcat(specs[currentSpec], drive);
			part++;
			break;
		case 1:
			if (dir && *dir)
			{
				char *spec = specs[currentSpec];
				if (strlen(specs[currentSpec]) != 0)
				{
					if (spec[strlen(specs[currentSpec]) - 1] == '\\' && *dir == '\\')
						dir++;
					else if (spec[strlen(specs[currentSpec]) - 1] != '\\' && drive && strlen(drive) > 2 && *dir != '\\')
						strcat(specs[currentSpec], "\\");
				}
				strcat(specs[currentSpec], dir);
				if (spec[strlen(specs[currentSpec]) - 1] != '\\')
					strcat(specs[currentSpec], "\\");
			}
			part++;
			break;
		case 2:
			if (name && *name)
			{
				if (*name == '\\')
					name++;
				char *found = NULL;
				char *last = NULL;
				char *start = name;
				while ((found = strstr(name, "\\")) != NULL)
					last = found;
				if (last)
				{
					*last = 0;
					strcat(dir, start);
					strcat(dir, "\\");
					name = last + 1;
				}
				strcat(specs[currentSpec], name);
			}
			part++;
			break;
		case 3:
			if (ext && *ext)
			{
				if (*ext != '.')
					strcat(specs[currentSpec], ".");
				strcat(specs[currentSpec], ext);
			}
			part++;
			break;
		}
	return specs[currentSpec];
}

// Splits path into the parts and, for a wildcard, finds its first match.
void FileSpec::become(char *path, int attrib)
{
	this->attrib = attrib;
	if (path)
	{
		fnsplit(path, drive, dir, name, ext);
		strcpy(foundName, name);
		strcpy(foundExt, ext);
	}
	else
		memset(this, 0, sizeof(FileSpec));
	done = 1;
	if (strstr(path, "*") || strstr(path, "?"))
	{
		done = findfirst(path, &found, this->attrib);
		if (done == 0)
		{
			if (found.ff_name[0] != '.')
			{
				char *dot = strstr(found.ff_name, ".");
				if (dot)
				{
					strcpy(ext, dot);
					*dot = 0;
				}
				else
					ext[0] = 0;
			}
			else
				ext[0] = 0;
			strcpy(name, found.ff_name);
		}
	}
}

FileSpec::FileSpec(void)
{
	memset(this, 0, sizeof(FileSpec));
	done = 1;
	attrib = 0;
}

FileSpec::FileSpec(char *drive, char *dir, char *name, char *ext)
{
	done = 1;
	attrib = 0;
	if (drive)
		strcpy(this->drive, drive);
	else
		this->drive[0] = 0;
	if (dir)
		strcpy(this->dir, dir);
	else
		this->dir[0] = 0;
	if (name)
	{
		strcpy(this->name, name);
		strcpy(foundName, name);
	}
	else
	{
		this->name[0] = 0;
		foundName[0] = 0;
	}
	if (ext)
	{
		strcpy(this->ext, ext);
		strcpy(foundExt, ext);
	}
	else
	{
		this->ext[0] = 0;
		foundExt[0] = 0;
	}
}

void FileSpec::unlinkAll(void)
{
	getFirst();
	getFirst();
	do
	{
		char *path = fileSpec(drive, dir, name, ext);
		unlink(path);
	} while (getNext());
}

int FileSpec::exists(void)
{
	return file_exists(fileSpec(drive, dir, name, ext));
}

// Moves to the next match with the wanted attribute; FALSE when none is left.
int FileSpec::getNext(void)
{
	if (done)
		return 0;
	unsigned char match = 0;
	do
	{
		done = findnext(&found);
		if (attrib == 0 || found.ff_attrib == attrib)
			match = 1;
	} while (!match && done == 0);
	if (!match)
		reset();
	else
	{
		if (found.ff_name[0] != '.')
		{
			char *dot = strstr(found.ff_name, ".");
			if (dot)
			{
				strcpy(ext, dot);
				*dot = 0;
			}
			else
				ext[0] = 0;
		}
		else
			ext[0] = 0;
		strcpy(name, found.ff_name);
	}
	return !done;
}

// TRUE if this file is newer than other's, or other's does not exist.
int FileSpec::isYounger(FileSpec &other)
{
	unsigned long mine, theirs;
	struct ffblk myFile, theirFile;

	if (findfirst(fileSpec(drive, dir, name, ext), &myFile, 0))
		return 0;
	if (findfirst(fileSpec(other.drive, other.dir, other.name, other.ext), &theirFile, 0))
		return 1;
	mine = ((unsigned long)myFile.ff_fdate << 16) | myFile.ff_ftime;
	theirs = ((unsigned long)theirFile.ff_fdate << 16) | theirFile.ff_ftime;
	return mine > theirs;
}

int FileSpec::isYounger(unsigned long date)
{
	unsigned long mine;
	struct ffblk myFile;

	if (findfirst(fileSpec(drive, dir, name, ext), &myFile, 0))
		return 0;
	mine = ((unsigned long)myFile.ff_fdate << 16) | myFile.ff_ftime;
	return mine > date;
}

// DOS date and time, date in the high word.
long FileSpec::getDate(void)
{
	struct ffblk file;

	if (findfirst(fileSpec(drive, dir, name, ext), &file, 0) == 0)
		return ((unsigned long)file.ff_fdate << 16) | file.ff_ftime;
	return 0;
}

void FileSpec::getFirst(void)
{
	reset();
	become(fileSpec(drive, dir, name, ext), attrib);
}

void FileSpec::clear(void)
{
	memset(drive, 0, sizeof(drive));
	memset(dir, 0, sizeof(dir));
	memset(name, 0, sizeof(name));
	memset(ext, 0, sizeof(ext));
	done = 1;
}

// Back to the name the search started from.
void FileSpec::reset(void)
{
	strcpy(name, foundName);
	strcpy(ext, foundExt);
}
