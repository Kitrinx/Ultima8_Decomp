// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: SPRITE.C

#include "ITEM.H"
#include "KERNEL.H"
#include "WAIT.H"
#include "SPRITE.H"

int createSprite(short shape, short first, short last, short end, short repeat, short delay, unsigned short x, unsigned short y, unsigned char z)
{
	Sprite *sprite = new Sprite(shape, first, last, end, repeat, delay, x, y, z);
	if (sprite->ref)
		return sprite->pid;
	sprite->pop(0);
	return 0;
}

int createSprite(short shape, short first, short last, short delay, unsigned short x, unsigned short y, unsigned char z)
{
	Sprite *sprite = new Sprite(shape, first, last, last, 1, delay, x, y, z);
	if (sprite->ref)
		return sprite->pid;
	sprite->pop(0);
	return 0;
}

Sprite::Sprite(unsigned short shape, unsigned short first, unsigned short last, unsigned short end, short times, unsigned short wait, unsigned short x, unsigned short y, unsigned char z)
{
	Kernel::setIdString(pid, "Sprite");
	Item item;
	if (item.create(shape, first))
	{
		setRef(item.referent);
		item.pop(x, y, z);
		item.setStatus(item.getStatus() | 0x80);
		if (isPhantom())
		{
			setRef(0);
			return;
		}
	}
	else
		setRef(0);
	firstFrame = first;
	frame = firstFrame;
	lastFrame = last;
	endFrame = end;
	forward = 1;
	turned = 0;
	repeat = times;
	delay = wait;
}

void Sprite::process(void)
{
	Item item(ref);

	if (turned)
	{
		turned = 0;
		if (forward)
		{
			if ((frame = lastFrame) < endFrame)
				frame++;
			else
				frame--;
			forward = 0;
			return;
		}
		repeat--;
		if (repeat)
		{
			if ((frame = firstFrame) < lastFrame)
				frame++;
			else
				frame--;
			forward = 1;
			return;
		}
		item.destroy();
		return;
	}
	item.setFrame(frame);
	if (forward)
	{
		if (firstFrame < lastFrame)
		{
			if (++frame > lastFrame)
				turned = 1;
		}
		else if (--frame < lastFrame)
			turned = 1;
	}
	else
	{
		if (lastFrame < endFrame)
		{
			if (++frame > endFrame)
				turned = 1;
		}
		else if (--frame < endFrame)
			turned = 1;
	}
	if (delay)
	{
		Wait *wait = new Wait(delay, ref);
		wait->then(this);
		wait->start();
	}
}
