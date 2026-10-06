// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: FILE\CFILE.C

#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys\stat.h>
#include "CFILE.H"
#include "CDESC.H"
#include "CEXIT.H"
#include "SYSTEM.H"

// Inline in the shared headers when this file was built.
inline SimpleVirtualString::SimpleVirtualString(void) { string = 0; size = 0; }
inline SimpleVirtualString::~SimpleVirtualString(void) { free(); }
inline SimpleVirtualString::operator char *(void) { return string; }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }

char *FileModeName[16] = {"ReadOnly", "WriteOnly", "ReadWrite", "NoShareReadOnly", "NoShareWriteOnly", "NoShareReadWrite", "ReadShareReadOnly", "ReadShareWriteOnly", "ReadShareReadWrite", "WriteShareReadOnly", "WriteShareWriteOnly", "WriteShareReadWrite", "RWShareReadOnly", "RWShareWriteOnly", "RWShareReadWrite", "Bad Mode"};

// Index of a mode in FileModeName.
int TranslateMode(OpenMode mode)
{
	int index;

	switch (mode)
	{
	case ReadOnly:				index = 0;	break;
	case WriteOnly:				index = 1;	break;
	case ReadWrite:				index = 2;	break;
	case NoShareReadOnly:		index = 3;	break;
	case NoShareWriteOnly:		index = 4;	break;
	case NoShareReadWrite:		index = 5;	break;
	case ReadShareReadOnly:		index = 6;	break;
	case ReadShareWriteOnly:	index = 7;	break;
	case ReadShareReadWrite:	index = 8;	break;
	case WriteShareReadOnly:	index = 9;	break;
	case WriteShareWriteOnly:	index = 10;	break;
	case WriteShareReadWrite:	index = 11;	break;
	case RWShareReadOnly:		index = 12;	break;
	case RWShareWriteOnly:		index = 13;	break;
	case RWShareReadWrite:		index = 14;	break;
	default:					index = 15;	break;
	}
	return index;
}

void BaseFile::fileError(int error, char *file, int line)
{
	char source[30];

	if (file)
		sprintf(source, "%s (%d)", file, line);
	else
		strcpy(source, "None");
	SimpleVirtualString message;
	message.assemble("Filename =%s\nMode     =%s\nError    =%04x\nDOS Error=%04x\nSource   =%s\n",
		filename, FileModeName[TranslateMode(mode)], error, fileerr, source);
	Fatal(message);
}

BaseFile::BaseFile(void)
{
	filename[0] = 0;
	clear();
}

BaseFile::BaseFile(const char *name, OpenMode mode)
{
	filename[0] = 0;
	clear();
	open(name, mode);
}

void BaseFile::clear(void)
{
	handle = -1;
	mode = ReadWrite;
}

void BaseFile::open(const char *name, OpenMode mode)
{
	strcpy(filename, name);
	this->mode = mode;
	if (handle == -1)
	{
		handle = SafeOpenFile(name, this->mode);
		if (fileerr)
			fileError(FILE_OPEN_FAILED);
	}
	else
		fileError(FILE_WRONG_STATE);
}

unsigned char BaseFile::eof(void)
{
	long pos = tell();
	seek(0, FromEnd);
	long end = tell();
	seek(pos, FromStart);
	return pos >= end;
}

// Opens a fresh copy of the file, creating it.
void BaseFile::forceOpen(const char *name, OpenMode mode)
{
	strcpy(filename, name);
	this->mode = mode;
	unlink(name);
	if (handle == -1)
	{
		handle = SafeOpenFile(name, this->mode);
		if (fileerr)
		{
			if (fileerr == 2)
			{
				handle = -1;
				create(name, mode);
			}
			else
				fileError(FILE_WRONG_STATE);
		}
		else
			fileError(FILE_WRONG_STATE);
	}
	else
		fileError(FILE_WRONG_STATE);
}

void BaseFile::remove(void)
{
	if (is_valid())
		close();
	if (::remove(filename))
		fileError(FILE_REMOVE_FAILED);
}

void BaseFile::rename(const char *name)
{
	if (handle == -1)
	{
		if (::rename(filename, name))
			fileError(FILE_RENAME_FAILED);
		strcpy(filename, name);
	}
	else
		fileError(FILE_RENAME_OPEN);
}

void BaseFile::openAppend(const char *name, OpenMode mode)
{
	strcpy(filename, name);
	this->mode = mode;
	if (handle == -1)
	{
		open(name, mode);
		if (fileerr)
		{
			if (fileerr == 2)
			{
				handle = -1;
				create(name, mode);
			}
			else
				fileError(FILE_WRONG_STATE);
		}
		else
			seek(0, FromEnd);
	}
	else
		fileError(FILE_WRONG_STATE);
}

void BaseFile::create(const char *name, OpenMode mode)
{
	strcpy(filename, name);
	this->mode = mode;
	if (handle == -1)
	{
		if (mode > NoShareReadWrite)
			handle = SafeCreateFile(name, 0x80);
		else
			handle = SafeCreateFile(name, 0);
		close();
		open(name, mode);
		if (fileerr)
			fileError(FILE_OPEN_FAILED);
	}
	else
		fileError(FILE_WRONG_STATE);
}

void BaseFile::close(void)
{
	if (is_valid())
	{
		CloseFile(handle);
		clear();
	}
}

long BaseFile::seek(long offset, SeekMode from)
{
	long pos = -1;

	if (is_valid())
		pos = SeekFile(handle, offset, from);
	if (fileerr)
		fileError(FILE_SEEK_FAILED);
	return pos;
}

// Reads count bytes at pos (-1: the current position). Buffers over 64K
// are read in pieces through the descriptor's linear address.
long BaseFile::read(char *buffer, long count, long pos)
{
	long total = -1;

	if (is_valid())
	{
		if (count > 0xffffL)
		{
			total = 0;
			unsigned long address = LocalDescriptorTable::table[FP_SEG(buffer) >> 3].getBase();
			long left = count;
			while (left != 0)
			{
				char *piece = (char *)makePtr(address);
				long length = left > 0xffdcL ? 0xffdcL : left;
				total += ReadFile(handle, pos, length, piece);
				address += length;
				if (pos != -1)
					pos += length;
				left -= length;
			}
		}
		else
			total = ReadFile(handle, pos, count, buffer);
	}
	else
		fileError(FILE_READ_FAILED);
	if (fileerr)
		fileError(FILE_READ_FAILED);
	return total;
}

// Reads one line of at most max - 1 characters and leaves the file after it.
int BaseFile::readString(char *buffer, short max, long pos)
{
	int i = 0;
	long start = tell();

	if (is_valid())
	{
		ReadFile(handle, pos, max, buffer);
		if (fileerr)
			fileError(FILE_READ_FAILED);
		while (buffer[i] != '\n' && i < max - 1)
			i++;
		if (i != max - 1)
		{
			buffer[i + 1] = 0;
			seek(start + i + 1, FromStart);
		}
		else
			buffer[max - 1] = 0;
	}
	else
		fileError(FILE_READ_FAILED);
	return i;
}

long BaseFile::write(char *buffer, long count, long pos)
{
	long total = -1;

	if (is_valid())
	{
		if (count > 0xffffL)
		{
			unsigned long address = LocalDescriptorTable::table[FP_SEG(buffer) >> 3].getBase();
			long left = count;
			while (left != 0)
			{
				char *piece = (char *)makePtr(address);
				long length = left > 0xffdcL ? 0xffdcL : left;
				WriteFile(handle, pos, length, piece);
				address += length;
				if (pos != -1)
					pos += length;
				left -= length;
			}
		}
		else if (WriteFile(handle, pos, count, buffer))
			total = count;
		else
			fileError(FILE_WRITE_FAILED);
	}
	else
		fileError(FILE_WRITE_FAILED);
	return total;
}

long BaseFile::size(void)
{
	long length = 0;

	if (is_valid())
	{
		long pos = SeekFile(handle, 0, FromCurrent);
		length = SeekFile(handle, 0, FromEnd);
		SeekFile(handle, pos, FromStart);
		if (fileerr)
			fileError(FILE_STAT_FAILED);
	}
	else
		fileError(FILE_STAT_FAILED);
	return length;
}

long BaseFile::getTime(void)
{
	unsigned time = 0;

	if (is_valid())
	{
		struct stat info;
		if (fstat(handle, &info))
			fileError(FILE_STAT_FAILED);
		time = info.st_atime;
	}
	else
		fileError(FILE_STAT_FAILED);
	return time;
}

char BaseFile::readable(void)
{
	char result = 0;

	if (is_valid())
	{
		struct stat info;
		if (fstat(handle, &info))
			fileError(FILE_STAT_FAILED);
		result = (info.st_mode & S_IREAD) == S_IREAD;
	}
	else
		fileError(FILE_STAT_FAILED);
	return result;
}

char BaseFile::writeable(void)
{
	char result = 0;

	if (is_valid())
	{
		struct stat info;
		if (fstat(handle, &info))
			fileError(FILE_STAT_FAILED);
		result = (info.st_mode & S_IWRITE) == S_IWRITE;
	}
	else
		fileError(FILE_STAT_FAILED);
	return result;
}

char BaseFile::executable(void)
{
	char result = 0;

	if (is_valid())
	{
		struct stat info;
		if (fstat(handle, &info))
			fileError(FILE_STAT_FAILED);
		result = (info.st_mode & S_IEXEC) == S_IEXEC;
	}
	else
		fileError(FILE_STAT_FAILED);
	return result;
}

// DOS commit file; carry set on failure.
int BaseFile::flush(void)
{
	_AH = 0x68;
	_BX = handle;
	geninterrupt(0x21);
	return !(_FLAGS & 1);
}

long BaseFile::tell(void)
{
	return SeekFile(handle, 0, FromCurrent);
}

// Copies count bytes from offset from in dest (this file if NULL) to
// offset to in this file, back to front when the ranges overlap forwards.
void BaseFile::relocate(long from, long to, long count, BaseFile *dest)
{
	if (count == 0)
		return;
	if (dest == NULL)
		dest = this;
	long distance = to - from;
	if (distance == 0 && dest == this)
		return;
	if (distance < 0)
		distance = -distance;
	unsigned char backwards = 0;
	unsigned length = count < 0xfe4fL ? (unsigned)count : 0xfe4f;
	if (distance < count && from < to && dest == this)
	{
		from = from + count - length;
		to = to + count - length;
		backwards = 1;
	}
	char *buffer = NULL;
	while (length)
	{
		buffer = new char[length];
		if (buffer)
			break;
		length >>= 1;
	}
	if (!length)
		outOfMemory(__FILE__, 951);	// __LINE__
	while (count)
	{
		dest->seek(from, FromStart);
		dest->read(buffer, length, -1);
		seek(to, FromStart);
		write(buffer, length, -1);
		count -= length;
		if (!backwards)
		{
			from += length;
			to += length;
			length = length > count ? count : length;
		}
		else
		{
			length = length > count ? count : length;
			from -= length;
			to -= length;
		}
	}
	delete buffer;
}

void BaseFile::setSize(long size)
{
	if (!is_valid())
		fileError(FILE_WRONG_STATE);
	if (chsize(handle, size))
		fileError(FILE_RESIZE_FAILED);
}

void BaseFile::setFilename(char *name)
{
	strcpy(filename, name);
}
