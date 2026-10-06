// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: INTER.C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "CEXIT.H"
#include "YAMM.H"
#include "UPROCESS.H"
#include "INTER.H"
#include "DEBUGGER.H"
#include "KERNEL.H"
#include "FLAG.H"
#include "SYSTEM.H"
#include "ITEMFIND.H"

unsigned unkDebug = 0;

int numToStr(unsigned short number)
{
	char text[10];
	itoa(number, text, 10);
	return Yamm::flimFlam(text);
}

long strToNum(char *text)
{
	char *p = text;
	return atol(p);
}

// The intrinsic called by number.
void *resolve(unsigned short number)
{
	if (number >= numCFunctions)
		halt(__FILE__, 69);	// __LINE__
	return ((void **)cTable)[number];
}

Data::Data(char *init, unsigned short size)
{
	sp = top = &buffer[200];
	bp = 199;
	if (init && size)
	{
		unsigned length = size;
		sp -= length;
		memcpy(sp, init, size);
	}
}

void Data::operator=(Data &from)
{
	memcpy(buffer, from.buffer, 200);
	sp = from.sp;
	top = from.top;
}

// Saves bp, points it at the new frame and clears the frame's locals.
void Data::makeFrame(int size)
{
	unsigned oldBp = bp;
	*(unsigned *)(sp -= 2) = oldBp;
	bp = sp - (char *)this;
	sp -= size;
	memset(&buffer[bp] - size, 0, size);
}

void Routine::jump(long offset)
{
	ip += (int)offset;
}

void Routine::jumpFar(long target)
{
	ip = RoutineIndex::getCode(target, target >> 16, 1, -1);
}

inline YammList::YammList(unsigned short l, unsigned short s, int str) { list = l; size = s; isString = str; }
inline void Yamm::setPid(unsigned short pos, unsigned short pid) { ((YammBlock *)(buffer + pos) - 1)->pid = pid; }
inline ProcessSel::ProcessSel(void) {}

// Runs the routine until it returns (1), suspends (2) or its process is killed (4).
int Routine::interpret(long &result)
{
	long l1;
	long l2;
	long temp;		// the usecode temp register
	int w1;
	int w2;
	int w3;
	int w4;
	int w5;
	char *s1;
	char *s2;
	char *s3;
	YammList source;
	YammList copy;
	unsigned savedPid;

	if (Yamm::currentUnkPid)
		savedPid = Yamm::currentUnkPid;
	else
		savedPid = 0;
	Yamm::currentUnkPid = process->pid;
	char *killed = process->killed;
	ProcessSel sels[10];
	int opcode;
	int done;

	if (debugger)
		debugger->reset();
	if (!suspended)
		pushLong(0);
	done = 0;
	while (!done)
	{
		opcode = *ip++;
		switch (opcode)
		{
		case 0x5a:	// allocate locals
			makeFrame(*ip++);
			break;
		case 0x5b:	// debug line
			int line;
			line = *((int *)ip)++;
			if (debugger)
				debugger->lineChange(line);
			break;
		case 0x5c:	// debug unit
			w1 = *((int *)ip)++;
			if (debugger)
				debugger->pushFile(ip, ip + w1, (char *)this + bp, 0);
			if (unkDebug)
			{
				UnitIndex *u;
				register unsigned i;

				gotoxy(1, 1);
				for (u = RoutineIndex::unitIndex, i = 0; i < MAX_UNITS; i++, u++)
					if (u->unit && u->useCount > 0)
						printf("%-8s\n\r", RoutineIndex::overloadTable[u->unit].name);
				gotoxy(1, 23);
				printf("Curr:%-8s", ip);
			}
			ip += 9;
			break;
		case 0x00:	// store local byte
			int offset;
			offset = *ip++;
			setLocalByte(offset, popWord());
			break;
		case 0x01:	// store local word
			offset = *ip++;
			setLocalWord(offset, popWord());
			break;
		case 0x02:	// store local dword
			offset = *ip++;
			setLocalLong(offset, popLong());
			break;
		case 0x03:	// store local bytes
			w1 = *ip++;
			w2 = *ip++;
			memcpy((char *)this + bp + w1, sp, w2);
			break;
		case 0x08:	// set process result
			result = popLong();
			break;
		case 0x4d:	// store pointer
			w1 = *ip++;
			s1 = (char *)popLong();
			memcpy(s1, sp, w1);
			reserve(-w1);
			break;
		case 0x74:	// search token
			pushByte(*ip++);
			break;
		case 0x0a:	// push s8
			pushWord(*ip++);
			break;
		case 0x0b:	// push u16
			pushWord(*((int *)ip)++);
			break;
		case 0x0c:	// push u32
			pushLong(*((long *)ip)++);
			break;
		case 0x3e:	// load local byte
			pushWord(localByte(*ip++));
			break;
		case 0x3f:	// load local word
			pushWord(localWord(*ip++));
			break;
		case 0x40:	// load local dword
			pushLong(localLong(*ip++));
			break;
		case 0x45:	// load local bytes
			w1 = *ip++;
			w2 = *ip++;
			s1 = sp;
			reserve(w2);
			memcpy(s1 - w2, (char *)this + bp + w1, w2);
			break;
		case 0x6d:	// load dependency result
			pushLong(result);
			break;
		case 0x4c:	// load pointer
			w1 = *ip++;
			s1 = (char *)popLong();
			s2 = sp;
			reserve(w1);
			memcpy(s2 - w1, s1, w1);
			break;
		case 0x4e:	// load global
			w1 = *((int *)ip)++;
			w2 = *ip++;
			pushWord(GlobalFlag::get(w1, w2));
			break;
		case 0x4f:	// store global
			w1 = *((int *)ip)++;
			w2 = *ip++;
			GlobalFlag::set(w1, w2, popWord());
			break;
		case 0x0e:	// make list
			w1 = *ip++;
			w2 = *ip++;
			w4 = 0;
			w5 = 0;
			for (w3 = 0; w3 < w2; w3++)
			{
				w4 = Yamm::flimFlam(w1 + 2);
				s1 = Yamm::buffer + w4;
				memcpy(s1 + 2, sp + w3 * w1, w1);
				*(int *)s1 = w5;
				w5 = w4;
			}
			reserve(-w1 * w2);
			pushWord(w4);
			break;
		case 0x42:	// load local list
			w1 = *ip++;
			w2 = *ip++;
			w3 = *(int *)((char *)this + bp + w1);
			source.set(w3, w2, 0);
			copy = source;
			pushWord(copy.list);
			copy.set(0, 0, 0);
			break;
		case 0x64:	// free local list
			YammList(localWord(*ip++), 0, 0).kill();
			break;
		case 0x44:	// load list element
			w1 = popWord();
			w2 = popWord();
			w3 = *ip++;
			w4 = *ip++;
			if (w3 == 1)
			{
				sp -= 2;
				*(int *)sp = 0;
			}
			else
				reserve(w3);
			YammList list(w2, w3, w4 & 1);
			s2 = (char *)list[w1 - 1];
			if (s2)
			{
				if (w4 & 1)
					*(int *)sp = Yamm::flimFlam(s2);
				else
					memcpy(sp, s2, w3);
			}
			else
				memset(sp, 0, w3);
			if (w4 & 2)
				list.kill();
			break;
		case 0x17:	// concat list
			w2 = popWord();
			w1 = popWord();
			YammList first(w1, 0, 0);
			YammList second(w2, 0, 0);
			first += w2;
			pushWord(first.list);
			break;
		case 0x66:	// free stack list
			w1 = *ip++;
			YammList stackList(*(int *)(sp + w1), 0, 0);
			stackList.kill();
			break;
		case 0x38:	// list contains
			w1 = *ip++;
			w4 = *ip++;
			w2 = popWord();
			s1 = sp;
			YammList searched(w2, w1, w4);
			w3 = searched.in(s1);
			searched.kill();
			if (w4)
				Yamm::scram(*(int *)sp);
			reserve(-w1);
			pushWord(w3);
			break;
		case 0x09:	// store list element
			w1 = popWord();
			w2 = *ip++;
			w3 = *ip++;
			w4 = *ip++;
			YammList stored(*(int *)((char *)this + bp + w2), w3, w4);
			stored.setElement(w1, sp);
			break;
		case 0x43:	// load local string list
			w1 = *ip++;
			w2 = *(int *)((char *)this + bp + w1);
			source.set(w2, 2, 1);
			copy = source;
			pushWord(copy.list);
			copy.set(0, 0, 0);
			break;
		case 0x63:	// free local string list
			YammList(localWord(*ip++), 0, 1).kill();
			break;
		case 0x18:	// xor list
		case 0x19:	// xor string list
		case 0x1a:	// subtract string list
		case 0x1b:	// subtract list
			w1 = *ip++;
			w3 = popWord();
			w2 = popWord();
			YammList left(w2, w1, opcode == 0x19 || opcode == 0x1a);
			YammList right(w3, w1, opcode == 0x19 || opcode == 0x1a);
			if (opcode == 0x1a || opcode == 0x1b)
				left -= right.list;
			else
				left ^= right.list;
			pushWord(left.list);
			right.kill();
			break;
		case 0x67:	// free stack string list
			YammList stringList(stackWord(*ip++), 0, 1);
			stringList.kill();
			break;
		case 0x0d:	// push string
			int length;
			length = *((int *)ip)++;
			w1 = Yamm::flimFlam(length + 1);
			if (!w1)
				halt(__FILE__, 504);	// __LINE__
			s1 = Yamm::buffer + w1;
			strncpy(s1, ip, length);
			s1[length] = 0;
			pushWord(w1);
			jump(length + 1);
			break;
		case 0x41:	// load local string
			w1 = *ip++;
			w2 = *(int *)((char *)this + bp + w1);
			s1 = Yamm::buffer + w2;
			w3 = Yamm::flimFlam(strlen(s1) + 1);
			s2 = Yamm::buffer + w3;
			strcpy(s2, s1);
			pushWord(w3);
			break;
		case 0x68:	// duplicate string
			w1 = popWord();
			s1 = Yamm::buffer + w1;
			w2 = strlen(s1);
			w3 = Yamm::flimFlam(w2 + 1);
			s2 = Yamm::buffer + w3;
			strncpy(s2, s1, w2 + 1);
			s2[w2] = 0;
			pushWord(w3);
			break;
		case 0x16:	// concat string
		case 0x26:	// equal string
			w2 = popWord();
			w1 = popWord();
			s1 = Yamm::buffer + w1;
			s2 = Yamm::buffer + w2;
			if (opcode == 0x26)
			{
				int unused;	// its block scope leaves an extra jump in the shipped code

				w3 = strcmp(s1, s2) == 0;
			}
			else
			{
				w4 = strlen(s1);
				w5 = strlen(s2);
				w3 = Yamm::flimFlam(w4 + w5 + 1);
				s3 = Yamm::buffer + w3;
				strcpy(s3, s1);
				strcat(s3, s2);
			}
			Yamm::scram(w2);
			Yamm::scram(w1);
			pushWord(w3);
			break;
		case 0x65:	// free stack string
			w1 = *ip++;
			Yamm::scram(*(int *)(sp + w1));
			break;
		case 0x62:	// free local string
			w1 = *ip++;
			Yamm::scram(*(int *)((char *)this + bp + w1));
			break;
		case 0x6a:	// pointer to string
			s1 = (char *)popLong();
			w2 = Yamm::flimFlam(s1);
			pushWord(w2);
			break;
		case 0x6b:	// string to pointer
			w1 = popWord();
			s1 = Yamm::buffer + w1;
			pushLong((long)s1);
			break;
		case 0x69:	// string local to pointer
			w1 = *ip++;
			pushLong((long)(Yamm::buffer + *(int *)((char *)this + bp + w1)));
			break;
		case 0x14:	// add word
			pushWord(popWord() + popWord());
			break;
		case 0x15:	// add dword
			pushLong(popLong() + popLong());
			break;
		case 0x1c:	// subtract word
			w2 = popWord();
			w1 = popWord();
			pushWord(w1 - w2);
			break;
		case 0x1d:	// subtract dword
			l2 = popLong();
			l1 = popLong();
			pushLong(l1 - l2);
			break;
		case 0x1e:	// multiply word
			pushWord(popWord() * popWord());
			break;
		case 0x1f:	// multiply dword
			pushLong(popLong() * popLong());
			break;
		case 0x20:	// divide word
			w2 = popWord();
			w1 = popWord();
			if (w2 == 0)
				pushWord(0);
			else
				pushWord(w1 / w2);
			break;
		case 0x21:	// divide dword
			l2 = popLong();
			l1 = popLong();
			if (l2 == 0)
				pushLong(0);
			else
				pushLong(l1 / l2);
			break;
		case 0x22:	// remainder word
			w2 = popWord();
			w1 = popWord();
			if (w2 == 0)
				pushWord(0);
			else
				pushWord(w1 % w2);
			break;
		case 0x23:	// remainder dword
			l2 = popLong();
			l1 = popLong();
			if (l2 == 0)
				pushLong(0);
			else
				pushLong(l1 % l2);
			break;
		case 0x24:	// equal word
			w1 = popWord();
			w2 = popWord();
			pushWord(w1 == w2);
			break;
		case 0x25:	// equal dword
			pushWord(popLong() == popLong());
			break;
		case 0x27:	// equal bytes
			w1 = *ip++;
			pushWord(memcmp(sp, sp + w1, w1) == 0);
			break;
		case 0x36:	// not equal word
			pushWord(popWord() != popWord());
			break;
		case 0x37:	// not equal dword
			pushWord(popLong() != popLong());
			break;
		case 0x28:	// less word (operands come off the stack in reverse)
			pushWord(popWord() > popWord());
			break;
		case 0x29:	// less dword
			pushWord(popLong() > popLong());
			break;
		case 0x2a:	// less or equal word
			pushWord(popWord() >= popWord());
			break;
		case 0x2b:	// less or equal dword
			pushWord(popLong() >= popLong());
			break;
		case 0x2c:	// greater word
			pushWord(popWord() < popWord());
			break;
		case 0x2d:	// greater dword
			pushWord(popLong() < popLong());
			break;
		case 0x2e:	// greater or equal word
			pushWord(popWord() <= popWord());
			break;
		case 0x2f:	// greater or equal dword
			pushWord(popLong() <= popLong());
			break;
		case 0x32:	// and word
			w1 = popWord();
			w2 = popWord();
			pushWord(w1 && w2);
			break;
		case 0x33:	// and dword
			l1 = popLong();
			l2 = popLong();
			pushWord(l1 && l2);
			break;
		case 0x34:	// or word
			w1 = popWord();
			w2 = popWord();
			pushWord(w1 || w2);
			break;
		case 0x35:	// or dword
			l1 = popLong();
			l2 = popLong();
			pushWord(l1 || l2);
			break;
		case 0x30:	// not word
			pushWord(!popWord());
			break;
		case 0x31:	// not dword
			pushWord(!popLong());
			break;
		case 0x39:	// bit and word
			pushWord(popWord() & popWord());
			break;
		case 0x3a:	// bit or word
			pushWord(popWord() | popWord());
			break;
		case 0x3c:	// shift left word
			pushWord(popWord() << popWord());
			break;
		case 0x3d:	// shift right word
			pushWord(popWord() >> popWord());
			break;
		case 0x3b:	// bit not word
			pushWord(~popWord());
			break;
		case 0x57:	// spawn
			if (debugger)
				debugger->stepDepth++;
			s1 = (char *)popLong();
			l1 = *ip++;
			w1 = *ip++;
			l2 = *((long *)ip)++;
			int pushed;
			Info saved;
			pushed = 0;
			if (debugger)
			{
				Info *info = debugger->getCurrentInfo();
				if (info)
				{
					pushed++;
					saved = *info;
					debugger->popFile();
				}
			}
			unsigned long childResult;
			UnkProcess *child;
			child = new UnkProcess(&childResult, s1, w1, (unsigned short)l2, (unsigned short)(l2 >> 16), 1, sp, (int)l1);
			if (debugger && pushed)
				debugger->pushFile(&saved);
			if (*killed)
			{
				delete killed;
				done = 4;
				goto out;
			}
			if (child->isPhantom())
			{
				temp = 0;
				result = childResult;
			}
			else
				temp = child->pid;
			break;
		case 0x58:	// spawn inline
			w1 = *((int *)ip)++;
			w2 = *((int *)ip)++;
			w3 = *((int *)ip)++;
			w4 = *ip++;
			w5 = *ip++;
			child = new UnkProcess(0, *(void **)((char *)this + bp + 6), w4, w1, w2 + w3, 1, (char *)this + bp + 10, w5);
			if (*killed)
			{
				delete killed;
				done = 4;
				goto out;
			}
			w1 = (char *)this + bp - sp;
			child->routine.reserve(w1);
			memcpy(child->routine.sp, sp, w1);
			w1 = child->isPhantom() ? 0 : child->pid;
			pushWord(w1);
			break;
		case 0x54:	// process dependency
			int count;
			int listSize;
			int k;
			count = *ip++;
			listSize = *ip++;
			for (k = 0; k < count; k++)
			{
				unsigned pid = popWord();
				register int j;
				register int dep;

				sels[k] = ProcessSel(pid);
				for (j = count + listSize - 1 - k; j > count - 1 - k; j--)
				{
					dep = ((int *)sp)[j - 1];
					Kernel::addDepElement(dep, pid);
				}
			}
			popWord();
			for (k = 0; k < count; k++)
				if (sels[k].isProcess())
					pushWord(sels[k].pid);
			break;
		case 0x55:	// and implies
			pushWord(DEP_AND);
			break;
		case 0x56:	// or implies
			pushWord(DEP_OR);
			break;
		case 0x53:	// suspend
			done = 2;
			suspended = 1;
			break;
		case 0x77:	// set process info
			w2 = popWord();
			w1 = popWord();
			process->setProcessType((ProcessType)w1);
			process->setRef(w2);
			break;
		case 0x78:	// exclude process
			w1 = Kernel::getNumProcesses(process->ref, process->processType);
			if (w1 > 1)
				done = 1;
			break;
		case 0x59:	// push pid
			pushWord(process->pid);
			break;
		case 0x6c:	// transfer ownership
			w1 = *ip++;
			w2 = *ip++;
			w3 = *(int *)((char *)this + bp + w1);
			w4 = process->pid;
			if (w2 == 1)
				Yamm::setPid(w3, w4);
			else if (w2 == 2)
			{
				YammList list(w3, 1, 0);
				list.changePid(w4);
			}
			else if (w2 == 3)
			{
				YammList list(w3, 0, 0);
				list.changePid(w4);
			}
			break;
		case 0x0f:	// call intrinsic
			int argBytes;
			void *(*intrinsic)(void);
			char *args;
			argBytes = *ip++;
			intrinsic = (void *(*)(void))resolve(*((unsigned short *)ip)++);
			_SP -= argBytes;
			args = (char *)MK_FP(_SS, _SP);
			memcpy(args, sp, argBytes);
			temp = (long)intrinsic();
			_SP += argBytes;
			if (*killed)
			{
				delete killed;
				done = 4;
				goto out;
			}
			break;
		case 0x10:	// call near
		case 0x11:	// call far
			if (debugger)
				debugger->stepDepth++;
			l1 = *((long *)ip)++;
			pushPtr(ip);
			if (opcode == 0x11)
				jumpFar(l1);
			else
				jump(l1);
			break;
		case 0x5d:	// load temp byte
			pushWord(temp & 0xff);
			break;
		case 0x5e:	// load temp word
			pushWord(temp);
			break;
		case 0x5f:	// load temp dword
			pushLong(temp);
			break;
		case 0x6e:	// adjust stack
			w1 = *ip++;
			reserve(w1);
			break;
		case 0x50:	// return
			sp = (char *)this + bp;
			bp = popWord();
			l1 = popLong();
			if (l1 == 0)
				done = 1;
			else
			{
				if (debugger)
				{
					if (debugger->currentInfo > 0)
						debugger->popFile();
					debugger->stepDepth--;
				}
				ip = (char *)l1;
			}
			break;
		case 0x70:	// search start
			w1 = *ip++;
			w2 = *ip++;
			w5 = *ip++;
			w3 = popWord();
			w4 = popWord();
			memcpy(WorkString.string, sp, w2);
			Item item(w3);
			int finderSize;
			ItemFinder *finder;
			switch (w5)
			{
			case 2:
				finder = new AreaItemFinder(item.getX(), item.getY(), w4, WorkString.string, w2);
				finderSize = sizeof(AreaItemFinder);
				break;
			case 3:
				finder = new RecursiveAreaItemFinder(item.getX(), item.getY(), w4, WorkString.string, w2);
				finderSize = sizeof(RecursiveAreaItemFinder);
				break;
			case 4:
				finder = new ContainerItemFinder(w4, WorkString.string, w2);
				finderSize = sizeof(ContainerItemFinder);
				break;
			case 5:
				finder = new RecursiveContainerItemFinder(w4, WorkString.string, w2);
				finderSize = sizeof(RecursiveContainerItemFinder);
				break;
			case 6:
				{
					int what;
					register Referent surface;

					if (w4 != -1 && w3 != -1)
					{
						what = 3;
						surface = w4;
					}
					else
					{
						if (w4 != -1)
						{
							what = 2;
							surface = w4;
						}
						if (w3 != -1)
						{
							what = 1;
							surface = w3;
						}
					}
					finder = new SurfaceItemFinder(surface, WorkString.string, w2, what);
					finderSize = sizeof(SurfaceItemFinder);
				}
			}
			if (!finder)
				outOfMemory(__FILE__, 1113);	// __LINE__
			reserve(finderSize - w2);
			memcpy(sp, finder, finderSize);
			pushWord(w1);
			w4 = finder->referent;
			*(int *)((char *)this + bp + w1) = w4;
			if (w4)
				((ItemFinder *)(sp + 2))->findNext();
			pushWord(w4);
			delete finder;
			break;
		case 0x73:	// search next
			finder = (ItemFinder *)(sp + 2);
			w1 = finder->referent;
			w2 = *(int *)sp;
			*(int *)((char *)this + bp + w2) = w1;
			pushWord(w1);
			if (w1)
				finder->findNext();
			break;
		case 0x75:	// iterate list
		case 0x76:	// iterate string list
			w2 = *ip++;
			w3 = *ip++;
			w4 = *((int *)ip)++;
			w1 = *(int *)sp;
			if (w1 == -1)
				w1 = ((int *)sp)[1];
			if (w1 == 0)
			{
				jump(w4);
				YammList(((int *)sp)[1], 0, opcode == 0x76).kill();
				popLong();
			}
			else
			{
				s1 = Yamm::buffer + w1;
				*(int *)sp = *(int *)s1;
				s2 = (char *)this + bp + w2;
				if (opcode == 0x76)
				{
					Yamm::scram(*(int *)s2);
					*(int *)s2 = Yamm::flimFlam(Yamm::buffer + ((int *)s1)[1]);
				}
				else
					memcpy((char *)this + bp + w2, s1 + 2, w3);
			}
			break;
		case 0x51:	// branch false
			l1 = popWord();
			w1 = *((int *)ip)++;
			if (l1 == 0)
				jump(w1);
			break;
		case 0x52:	// branch
			jump(*((int *)ip)++);
			break;
		case 0x60:	// word to dword
			pushLong(popWord());
			break;
		case 0x61:	// dword to word
			pushWord(popLong());
			break;
		case 0x6f:	// push stack pointer
			pushPtr(stackPtr(-*ip++));
			break;
		case 0x4b:	// push local pointer
			pushPtr(localPtr(*ip++));
			break;
		case 0x12:	// set temp word
			temp = popWord();
			break;
		case 0x13:	// set temp dword
			temp = popLong();
			break;
		default:
			halt(__FILE__, 1265, "Undefined token %d\n", opcode);	// __LINE__
			break;
		}
	}
out:
	Yamm::currentUnkPid = savedPid;
	if (debugger)
	{
		if (debugger->currentInfo > 0)
			debugger->popFile();
		debugger->stepDepth--;
		debugger->graphicVideoMode();
		debugger->reset();
	}
	return done;
}
