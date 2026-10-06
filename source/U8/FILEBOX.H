#ifndef FILEBOX_H
#define FILEBOX_H

#include <dir.h>
#include "BASECAM.H"
#include "COMBOBOX.H"
#include "FILESPEC.H"
#include "LISTBOX.H"

class Event;
class NewGump;
class NewGumpId;

class FileBox : public ComboBox
{
public:
	char fileSpec[80];

	FileBox(NewGump *, char *);
	virtual void commandPrivate(Event &);
	virtual void init(void);
	virtual void unInit(void);
	void setFileSpec(char *);
	void setDrive(char);
	void setWild(char *);
	char *getDesc(void);
};

class DirectoryBox : public ListBoxGump
{
public:
	char **dirs;

	DirectoryBox(NewGump *, char *);
	virtual void commandMouseLeft(Event &);
	unsigned char setDir(char *);
};

class FileSelector : public PanelGump
{
public:
	char path[MAXDIR];
	int drive;
	FileSpec spec;
	FileSpec *result;
	char driveNames[26][2];
	char *names[6];		// the caller's buttons

	FileSelector(NewGumpId, NewGump *, char *, FileSpec *, ...);
	virtual ~FileSelector(void);
	virtual void commandPrivate(Event &);
	virtual void draw(short, short);
	unsigned char buildResult(void);
};

#endif
