// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: BARKFIDG.C

#include "NPC.H"
#include "KERNEL.H"
#include "DISPATCH.H"
#include "BARK.H"
#include "STATREST.H"
#include "WAIT.H"
#include "BARKFIDG.H"

BarkFidget::BarkFidget(unsigned short npc, unsigned char x)
{
	setRef(npc);
	setProcessType(0x227);
	Kernel::setIdString(pid, "BarkFidget");
	f_2e = x;
	intFlags |= 1;
}

Boolean BarkFidget::hasBark(void)
{
	NewGump *top = Dispatcher::base[Dispatcher::baseSP];
	BarkGump *bark = 0;
	while (top->newGumpList.hasType(0x43, (NewGump *&)bark))
		if (bark->f_52 == 2 && bark->item == ref)
			return TRUE;
	return FALSE;
}

void BarkFidget::process(void)
{
	Npc npc(ref);
	if (npc.getLastAnimSet() != ANIM_STAND && npc.getLastAnimSet() != ANIM_TALK)
	{
		pop(0);
		return;
	}
	if (npc.isBusy())
		return;
	if (hasBark())
	{
		int type = npc.getType();
		int animPid = 0;
		if (AnimCache::isAnimInExistence(type, ANIM_TALK))
			animPid = npc.doAnim(ANIM_TALK, 8, 1, 0);
		if (animPid)
		{
			Process *anim;
			Wait *wait = new Wait(rndRange(1, 3) * 30, ref);
			anim = Kernel::getProcess(animPid);
			anim->then(wait)->then(this);
			wait->notifyOfDeath(pid);
			wait->start();
			return;
		}
		pop(0);
		return;
	}
	pop(0);
}

void BarkFidget::interruptHandler(unsigned long, unsigned long)
{
	if (dependencyFailed())
		pop(0);
}
