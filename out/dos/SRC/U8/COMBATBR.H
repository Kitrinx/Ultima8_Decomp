#ifndef COMBATBR_H
#define COMBATBR_H

#include "..\GLIB\PROCESS.H"
#include "MLIST.H"

class TypeChanger : public Process
{
public:
	unsigned newType;

	TypeChanger(unsigned short, unsigned short);
	virtual void process(void);
};

class WaitForVictim : public ProcessInterrupt
{
public:
	unsigned char victimFound;	// set when the wait runs out
	unsigned range;
	unsigned waitPid;

	WaitForVictim(unsigned short, short);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
};

class CombatBrain : public ProcessInterrupt
{
public:
	unsigned target;
	unsigned fixedTarget;
	unsigned shiftWaitPid;	// a hidden changeling waits on it before coming out
	unsigned char f_34;
	unsigned char f_35;
	unsigned char f_36;
	unsigned char changeling;	// type 357, which hides as type 1

	CombatBrain(unsigned short, unsigned short);
	virtual void process(void);
	virtual void interruptHandler(unsigned long, unsigned long);
	int getTarget(void);
	void setTarget(unsigned short);
	void lockTarget(unsigned short);
	unsigned char selectTarget(short);
	unsigned char canHitTarget(void);
	void attackTarget(void);
	void gotoTarget(void);
	unsigned char isFacingTarget(void);
	unsigned char faceTarget(void);
	unsigned char hasTarget(void);
	void wander(unsigned char);
};

class DataTable
{
public:
	static int getNumMonsterRecords(void);
	static int getMonsterRecord(unsigned short);
	static unsigned short monsterTypes[31];
	static unsigned char data[416];
};

unsigned char isFeignable(unsigned short);

#endif
