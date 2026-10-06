// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: GAMETIME.C

#include "KERNEL.H"
#include "ITEMCACH.H"
#include "CAMERA.H"
#include "GAMETIME.H"

#define TICKS_PER_SECOND	30
#define TICKS_PER_HOUR		27000L

GameTime *theGameTime = 0;
Animation *theAnimation = 0;

inline void GameTime::postLoad(void) { theGameTime = this; }
inline void startSchedules(unsigned short hour) { new Scheduler(hour); }
inline void Animation::postLoad(void) { theAnimation = this; }

GameTime::GameTime(void)
{
	loops = 0;
	pause = 0;
	hourChanged = 0;
	setProcessType((ProcessType)0x245);
	Kernel::setIdString(pid, "GameTime");
	setDaemon();
}

void GameTime::process(void)
{
	if (hourChanged)
	{
		hourChanged = 0;
		startSchedules(theAnimation->getHours());
	}
	if (pause)
	{
		if (pause != -1)
			pause--;
	}
	else
		loops++;
}

Animation::Animation(void)
{
	f_3c = f_38 = f_40 = f_44 = 0;
	f_48 = -1;
	f_4c = f_4d = 0;
	Kernel::setIdString(pid, "Animation");
	setProcessType((ProcessType)0x230);
	setSafeInterrupt();
	setHertz(TICKS_PER_SECOND);
	setIntFlags(intFlags | 2);
	setDaemon();
}

void Animation::interruptHandler(unsigned long event, unsigned long)
{
	switch (event)
	{
	case 2:
		if (f_48 >= 0)
		{
			if (f_40 / TICKS_PER_HOUR != f_48 / TICKS_PER_HOUR)
			{
				f_44 = f_48 / TICKS_PER_HOUR;
				theGameTime->hourChanged = 1;
			}
			f_40 = f_48;
			f_48 = -1;
		}
		else
		{
			f_40++;
			if (f_40 % TICKS_PER_HOUR == 0)
			{
				f_44++;
				theGameTime->hourChanged = 1;
			}
		}
		if (!theGameTime->isPaused() && !f_4d)
		{
			f_38++;
			if (!f_4c)
				f_3c = f_38;
			Camera::needAnim++;
		}
		break;
	}
}

void Animation::lockTime(void)
{
	f_4c = 1;
}

void Animation::unlockTime(void)
{
	f_3c = f_38;
	f_4c = 0;
}

void initGameTime(void)
{
	if (!theGameTime)
		theGameTime = new GameTime;
	if (!theAnimation)
	{
		theAnimation = new Animation;
		theAnimation->connectToRealTime(1);
	}
}

long TimeInGameLoops(void)
{
	return theGameTime->loops;
}

long TimeInSeconds(void)
{
	return theAnimation->f_40 / TICKS_PER_SECOND;
}

long TimeInMinutes(void)
{
	return theAnimation->f_40 / (60 * TICKS_PER_SECOND);
}

long TimeInGameHours(void)
{
	return theAnimation->f_44;
}

void SetTimeInSeconds(long seconds)
{
	theAnimation->setTicks(seconds * TICKS_PER_SECOND);
}

void SetTimeInMinutes(long minutes)
{
	theAnimation->setTicks(minutes * (60 * TICKS_PER_SECOND));
}

void SetTimeInGameHours(short hours)
{
	theAnimation->setTicks(hours * TICKS_PER_HOUR);
}
