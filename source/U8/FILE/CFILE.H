#ifndef CFILE_H
#define CFILE_H

// DOS open modes: access in the low nibble, sharing in the high one.
enum OpenMode
{
	ReadOnly = 0x00,
	WriteOnly = 0x01,
	ReadWrite = 0x02,
	NoShareReadOnly = 0x10,
	NoShareWriteOnly = 0x11,
	NoShareReadWrite = 0x12,
	ReadShareReadOnly = 0x20,
	ReadShareWriteOnly = 0x21,
	ReadShareReadWrite = 0x22,
	WriteShareReadOnly = 0x30,
	WriteShareWriteOnly = 0x31,
	WriteShareReadWrite = 0x32,
	RWShareReadOnly = 0x40,
	RWShareWriteOnly = 0x41,
	RWShareReadWrite = 0x42
};

enum SeekMode
{
	FromStart,
	FromCurrent,
	FromEnd
};

// fileError codes
#define FILE_OPEN_FAILED	0x1910
#define FILE_WRONG_STATE	0x1911
#define FILE_WRITE_FAILED	0x1920
#define FILE_READ_FAILED	0x1930
#define FILE_SEEK_FAILED	0x1940
#define FILE_STAT_FAILED	0x1950
#define FILE_REMOVE_FAILED	0x1961
#define FILE_RENAME_FAILED	0x1962
#define FILE_RENAME_OPEN	0x1963
#define FILE_RESIZE_FAILED	0x1965

class BaseFile
{
public:
	char filename[80];
	int handle;
	OpenMode mode;

	BaseFile(void);
	BaseFile(const char *, OpenMode);
	~BaseFile(void);
	virtual long read(char *, long, long = -1);
	virtual long write(char *, long, long = -1);
	virtual void fileError(int, char * = 0, int = -1);
	void clear(void);
	void open(const char *, OpenMode);
	unsigned char eof(void);
	void forceOpen(const char *, OpenMode);
	void remove(void);
	void rename(const char *);
	void openAppend(const char *, OpenMode);
	void create(const char *, OpenMode);
	void close(void);
	long seek(long, SeekMode = FromStart);
	int readString(char *, short, long = -1);
	long size(void);
	long getTime(void);
	char readable(void);
	char writeable(void);
	char executable(void);
	int flush(void);
	long tell(void);
	void relocate(long, long, long, BaseFile * = 0);
	void setSize(long);
	void setFilename(char *);
	unsigned char is_valid(void);
};

int TranslateMode(OpenMode);

extern char *FileModeName[16];

// DOS file routines; each sets fileerr to the DOS error, or 0.
extern "C" {
int SafeOpenFile(const char *, int);
int SafeCreateFile(const char *, int);
int CloseFile(int);
long ReadFile(int, long, long, void *);
char WriteFile(int, long, long, void *);
long SeekFile(int, long, int);
unsigned char file_exists(const char *);
extern int fileerr;
}

#endif
