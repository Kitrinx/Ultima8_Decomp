#ifndef FLAG_H
#define FLAG_H

// Usecode global flags: a 1024-byte bit array saved as flag.dat, with
// flagsym.dat naming each flag's bit position and width.
class GlobalFlag
{
public:
	static int get(int, int);
	static void set(int, int, int);
	static void destroy(void);
	static void load(char *);
	static void save(char *);
	static char findFlag(char *, int &, int &);
	static char *flagSym;
	static int flagSymSize;
	static char *bits;
};

extern char *flagFileName;

#endif
