#ifndef MAIN_H
#define MAIN_H

#include "STEPFLEX.H"

class CommandLineOptions;

class PatchFlexFile : public StepFlexFile
{
public:
	PatchFlexFile(char *name) : StepFlexFile(name, -1, 0) {}
	void grabHeader(void);
};

void kernelDump(short, short);
void displayHelp(void);
void performCheatCheck(char *);
void parseCommandLine(int, char **, CommandLineOptions &);
void mainLoopFunction(void);
void checkForPatchFiles(void);

extern unsigned cheatcheck1;
extern unsigned long cheatcheck2;
extern unsigned long cheatcheck3;
extern char DateOfBuild[12];
extern char TimeOfBuild[9];
extern char Version[5];
extern char UsecodeVersion[5];
extern char IntroVersion[5];
extern char EndgameVersion[5];
extern char SpeechVersion[5];
extern char Flavor[7];
extern unsigned _stklen;
extern unsigned char gameLoopCounter[1];
extern unsigned char inGameMode;
extern char *validArgs[11];
extern char *patchPaths[4];

#endif
