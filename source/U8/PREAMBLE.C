// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: PREAMBLE.C

#include <string.h>
#include "KERNEL.H"
#include "PRIORITY.H"
#include "NPC.H"
#include "SCRIT.H"
#include "CAMERA.H"
#include "PAL.H"
#include "PAGAN.H"
#include "FLICPLAY.H"
#include "U8DISPAT.H"
#include "DSFXMAN.H"
#include "FEXIST.H"
#include "FILESPEC.H"
#include "SCRATCHM.H"
#include "ITEMCACH.H"
#include "EGG.H"
#include "GAMETIME.H"
#include "PREAMBLE.H"

RestoreMusicProcess::RestoreMusicProcess(void)
{
	Kernel::setIdString(pid, "RestoreMusic");
	theCameraProcess->flags |= PROC_SUSPENDED;
}

RestoreMusicProcess::~RestoreMusicProcess(void)
{
	theCameraProcess->flags &= ~PROC_SUSPENDED;
}

void RestoreMusicProcess::process(void)
{
	restoreMusic();
	pop(0);
}

void RestoreMusicProcess::fail(long result)
{
	restoreMusic();
	Process::fail(result);
}

PreambleProcess::PreambleProcess(void) : f_2a(0)
{
	Kernel::setIdString(pid, "Preamble");
	setProcessType((ProcessType)0x239);
	char named = avatar.name[0];
	char flic[80];

	strcpy(flic, introFileName);
	flic[0] = avatar.getLanguageChar();
	if (!FileExists(fileSpec(NULL, StaticDir, flic, NULL)))
		flic[0] = introFileName[0];
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	Process *restore = new RestoreMusicProcess;
	if (!named)
	{
		// A new game: center on the starting egg and equip the avatar with item type 529.
		Kernel::resetRef(avatar.referent, (ProcessType)6);
		setAvatarInStasis(1);
		startEgg.init(0, 0, 0xffff, 0xffff, SearchCriteria(':', '%', 4, '=', '*', '%', 36, '=', '&', '$'), 0xffff);
		if (startEgg.isValid())
		{
			unsigned char priority = Kernel::currentPriority;
			Kernel::setCurrentPriority(1);
			Camera::setCenterOn(0);
			Camera::move_to(startEgg.getX(), startEgg.getY(), startEgg.getZ(), ItemCache::mapIn);
			Kernel::setCurrentPriority(priority);
		}
		avatar.music = 52;
		Item shirt;
		avatar.destroyContents();
		avatar.clearEquip();
		shirt.create(529, 0);
		shirt.popToEnd(avatar.referent);
		avatar.setEquip(6, shirt.referent);
		shirt.equip();
		(new MainMenuProcess)->then(restore)->then(this);
		Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	}
	if (theCameraProcess)
	{
		theCameraProcess->flags |= PROC_SUSPENDED;
		theCameraProcess->flags &= ~PROC_DAEMON;
	}
	theU8Dispatcher->flags |= PROC_SUSPENDED;
	theU8Dispatcher->flags &= ~PROC_DAEMON;
	(new FadeProcess(RGB(0, 0, 0), 0x7fff, 1))->then(new RefreshWorld);
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	new CoolKeyboard;
	FlicPlayer *player = new FlicPlayer(fileSpec(NULL, StaticDir, flic, NULL), 0, 0);
	player->then(restore);
}

// Kills the type 6 processes of a priority class, then drops to priority 1.
inline void killType6(PriorityClass priority)
{
	Kernel::killProcess(0, (ProcessType)6, priority);
	Kernel::setCurrentPriority(1);
}

void PreambleProcess::process(void)
{
	theGameTime->pause = 0;
	if (startEgg.isValid())
	{
		killType6(PriorityClass(Kernel::currentPriority | 0x20));
		theU8Dispatcher->setDaemon();
		Egg egg(startEgg.referent);
		egg.hatch();
		startEgg.referent = 0;
	}
	else
	{
		killType6(PriorityClass(Kernel::currentPriority | 0x20));
	}
}

void preamble(void)
{
	Camera::position();
	Kernel::setCurrentPriority(Kernel::currentPriority + 1);
	new PreambleProcess;
}
