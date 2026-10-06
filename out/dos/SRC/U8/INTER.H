#ifndef INTER_H
#define INTER_H

// A usecode process's stack; it grows down from the top of the buffer.
class Data
{
public:
	char buffer[200];
	char *sp;
	char *top;
	unsigned bp;

	Data(char *, unsigned short);
	void operator=(Data &);
	void makeFrame(int);

	int popWord(void) { int v = *(int *)sp; sp += 2; return v; }
	long popLong(void) { long v = *(long *)sp; sp += 4; return v; }
	void pushByte(char v) { *(sp -= 1) = v; }
	void pushWord(int v) { *(int *)(sp -= 2) = v; }
	void pushLong(long v) { *(long *)(sp -= 4) = v; }
	void pushPtr(char *p) { *(char **)(sp -= 4) = p; }
	void reserve(int size) { sp -= size; }
	char *stackPtr(int offset) { return sp + offset; }
	int stackWord(int offset) { return *(int *)(sp + offset); }

	// Locals live at negative offsets from bp, arguments at positive ones.
	char *localPtr(int offset) { return (char *)this + bp + offset; }
	char localByte(int offset) { return ((char *)this + bp)[offset]; }
	int localWord(int offset) { return *(int *)((char *)this + bp + offset); }
	long localLong(int offset) { return *(long *)((char *)this + bp + offset); }
	void setLocalByte(int offset, char v) { ((char *)this + bp)[offset] = v; }
	void setLocalWord(int offset, int v) { *(int *)((char *)this + bp + offset) = v; }
	void setLocalLong(int offset, long v) { *(long *)((char *)this + bp + offset) = v; }
};

class Debugger;
class UnkProcess;

class Routine : public Data
{
public:
	char *ip;
	UnkProcess *process;
	unsigned char pad_da[2];
	unsigned char suspended;	// set by a suspend: resuming skips pushing the base return address

	Routine(char *init, unsigned short size, char *code, UnkProcess *owner) : Data(init, size)
	{
		ip = code;
		suspended = 0;
		process = owner;
	}
	void jump(long);
	void jumpFar(long);
	int interpret(long &);
};

// One word of a dependency list: a pid or a DEP_ operator.
class ProcessSel
{
public:
	unsigned pid;

	ProcessSel(void);
	ProcessSel(unsigned short p) { pid = p; }
	unsigned char isOperator(void) { return (pid & 0x400) ? 1 : 0; }
	unsigned char isProcess(void) { return !isOperator(); }
};

int numToStr(unsigned short);
long strToNum(char *);
void *resolve(unsigned short);

extern Debugger *debugger;
extern unsigned unkDebug;

#endif
