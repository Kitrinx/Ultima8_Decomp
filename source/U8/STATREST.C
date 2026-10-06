// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: STATREST.C

#include "..\GLIB\PROCESS.H"
#include "KERNEL.H"
#include "NPC.H"
#include "MAIN.H"
#include "STATREST.H"

inline void Avatar::setEffectiveStr(int s) { effectiveStr = s; }

StatRestorer::StatRestorer(void)
{
	Kernel::setIdString(pid, "StatRestorer");
	setProcessType((ProcessType)0x222);
	setHertz(1);
	flags |= PROC_SUSPENDED;
	setDaemon();
	ticks = 0;
	intFlags |= 2;
	connectToRealTime(1);
}

// Once a second: mana every 20 seconds; hit points and hunger every 35.
void StatRestorer::process(void)
{
	int value;
	unsigned char current, max;

	if (!inGameMode)
		return;
	if (ticks % 20 == 0)
	{
		current = avatar.getMana();
		max = avatar.getInt();
		if ((value = current) < max * 2)
			avatar.setMana(value + 1);
	}
	if (ticks == 35)
	{
		current = avatar.getHp();
		max = avatar.getStr();
		if (current < max * 2)
			avatar.setHp(current + 1);
		int hunger = avatar.effectiveStr;
		if (hunger < 200)
			avatar.setEffectiveStr(hunger + 1);
		ticks = 0;
	}
	flags |= PROC_SUSPENDED;
	intFlags |= 2;
}

void StatRestorer::interruptHandler(unsigned long event, unsigned long)
{
	if (!inGameMode)
		return;
	switch (event)
	{
	case 2:
		intFlags &= ~2L;
		ticks++;
		flags &= ~PROC_SUSPENDED;
		break;
	}
}

// Eating lowers the hunger counter and heals a quarter of the drop as hit points.
void FeedAvatar(short amount)
{
	int hunger = avatar.effectiveStr;

	if (hunger == 0)
		return;
	if (amount > hunger)
		amount = hunger;
	int heal = (hunger >> 2) - ((hunger - amount) >> 2);
	unsigned char str = avatar.getStr();
	if (heal && avatar.getHp() < str * 2)
	{
		unsigned char hp = avatar.getHp() + heal;
		avatar.setHp(hp > str * 2 ? str * 2 : hp);
	}
	avatar.setEffectiveStr(hunger - amount);
}

// Strength fades as hunger climbs from 150 to 200.
char Avatar::getEffectiveStr(void)
{
	if (effectiveStr >= 150)
		return getStr() * (200 - effectiveStr) / 50;
	return getStr();
}

inline void Process::receiveMessage(Message, unsigned short, long)
{
}

inline void Process::postLoad(void)
{
}

inline unsigned char ProcessInterrupt::checkInterrupt(unsigned long, unsigned long &)
{
	return 0;
}
