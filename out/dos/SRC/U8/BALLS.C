// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: BALLS.C

#include "NPC.H"
#include "MOTRAK.H"
#include "ERROR.H"
#include "KERNEL.H"
#include "SCRATCHM.H"
#include "STATREST.H"
#include "WAIT.H"
#include "BALLS.H"

Explode::Explode(unsigned short r)
{
	Item item(r);
	setRef(r);
	count = 1;
	item.setType(261);
	switch (item.getFrame()) {
	case 0:
		item.setFrame(5);
		break;
	case 1:
	case 4:
		item.setFrame(15);
		break;
	case 2:
		item.setFrame(25);
		break;
	case 3:
	case 5:
		item.setFrame(35);
		break;
	}
}

void Explode::process(void)
{
	Item item(ref);
	count++;
	if (count > 7)
		item.destroy();
	else
		item.setFrame(item.getFrame() + 1);
}

Ball::Ball(BallVariety v, unsigned short t, char speed)
{
	variety = v;
	target = t;
	maxSpeed = speed;
	setProcessType((ProcessType)0x218);
	dx = 0;
	dy = 0;
	dz = 0;
	children = 0;
	numChildren = 0;
}

// Places the ball (and its tail) at the first clear spot near x, y.
unsigned char Ball::init(BallType t, unsigned short x, unsigned short y, unsigned short z)
{
	Item ball;
	MotionTracker tracker;
	WorldPoint pt;
	unsigned startX;
	unsigned startY;

	type = t;
	x -= 100;
	y -= 100;
	if (variety == 1 || variety == 3)
		z += 35;
	startX = x;
	startY = y;
	ball.create(260, type);
	setRef(ball.referent);
	pt.set(x, y, z);
	while (!tracker.isClear(260, pt, pt, 4, ref, 0, 0)) {
		x += 25;
		if (startX + 200 == x) {
			x = startX;
			y += 25;
		}
		if (startY + 200 == y) {
			ball.destroy();
			return 0;
		}
		pt.set(x, y, z);
	}
	ball.pop(x, y, z);
	if (variety == 2 || variety == 3) {
		if (type == 4)
			numChildren = 3;
		else
			numChildren = 5;
		children = new unsigned short[numChildren];
		if (children == 0)
			halt(__FILE__, 147);	// __LINE__
		for (int i = 0; i < numChildren; i++) {
			Item child;
			child.create(261, type == 4 ? i + 1 : type);
			child.pop(ball.getX() + 64, ball.getY(), ball.getZ());
			children[i] = child.referent;
		}
	}
	return 1;
}

void Ball::process(void)
{
	Referent hit;
	Item ball(ref);
	int frame = ball.getFrame();
	Item victim(target);

	if (variety == 2 || variety == 3) {
		for (int i = numChildren - 1; i >= 0; i--) {
			Item child(children[i]);
			if (i != 0) {
				Item ahead(children[i - 1]);
				child.move(ahead.getX(), ahead.getY(), ahead.getZ());
			} else
				child.move(ball.getX(), ball.getY(), ball.getZ());
		}
	}
	if (ball.getFrame() == 3)
		generateChild();
	if (ball.getFrame() == 0 && OneIn(600))
		ball.setFrame(1);
	if ((hit = cantMove()) != 0) {
		Item other(hit);
		if (other.isNpc()) {
			if (frame == 1 || frame == 4) {
				victim.receiveHit(ref, 8, rndRange(5, 10), 8);
				explode();
			} else if (frame == 2)
				generateChild();
			else
				reflect();
		} else if (other.getType() == 260) {
			if (frame == 1 || frame == 4 || other.getFrame() == 1 || other.getFrame() == 4) {
				Ball *ball = (Ball *)Kernel::findValidProcess(other.referent, (ProcessType)0x218);
				if (ball)
					ball->explode();
				else
					halt(__FILE__, 221);	// __LINE__
				explode();
			} else
				reflect();
		} else if (OneIn(20))
			explode();
		else
			reflect();
	}
}

// Moves the ball one step; returns what it hit.
int Ball::cantMove(void)
{
	Item ball(ref);
	MotionTracker tracker;
	WorldPoint from;
	WorldPoint to;
	Item victim(target);
	int z;
	Boolean bounced = 0;

	addSpeed();
	z = ball.getZ() + dz;
	if (z < 0) {
		bounced = 1;
		z = 0;
		dz = -dz;
	} else if (z >= 240) {
		bounced = 1;
		z = 240;
		dz = -dz;
	}
	from.set(ball.getX(), ball.getY(), ball.getZ());
	to.set(from.x + dx, from.y + dy, z);
	if (tracker.isClear(ball.getType(), from, to, 4, ref, 0, 0))
		ball.move(to);
	else if (variety == 1 || variety == 3) {
		to.z = ball.getZ();
		if (tracker.isClear(ball.getType(), from, to, 4, ref, 0, 0)) {
			if (!bounced)
				dz = -dz;
			ball.move(to);
		}
	}
	return tracker.hitItem;
}

void Ball::generateChild(void)
{
	Item ball(ref);
	if (ball.getFrame() == 3) {
		Ball *child = new Ball(variety, 1, 25);
		if (!child->init((BallType)(OneIn(2) ? 2 : 1), ball.getX(), ball.getY(), ball.getZ()))
			child->fail(0);
		else
			ball.setFrame(0);
	} else {
		ball.setFrame(3);
		Wait *wait = new Wait(90, 0);
		wait->then(this);
		wait->start();
		dx = 0;
		dy = 0;
	}
}

void Ball::reflect(void)
{
	dx = -dx;
	dy = -dy;
}

// Steers the ball toward its target, up to its top speed.
void Ball::addSpeed(void)
{
	Npc ball(ref);
	char dir;
	Item victim(target);
	dir = ball.getDirToCoords(victim.getX(), victim.getY());
	dx += x8add[dir];
	dy += y8add[dir];
	if (dx > maxSpeed)
		dx = maxSpeed;
	else if (dx < -maxSpeed)
		dx = -maxSpeed;
	if (dy > maxSpeed)
		dy = maxSpeed;
	else if (dy < -maxSpeed)
		dy = -maxSpeed;
	if (variety == 1 || variety == 3) {
		if (victim.getZ() > ball.getZ())
			dz += 1;
		else
			dz -= 1;
	}
}

void Ball::explode(void)
{
	new Explode(ref);
	for (int i = 0; i < numChildren; i++)
		new Explode(children[i]);
}

void TonysBalls(short type, short variety, unsigned short x, unsigned short y, unsigned short z)
{
	Ball *ball = new Ball((BallVariety)variety, avatar.referent, 25);
	if (!ball->init((BallType)type, x, y, z))
		ball->fail(0);
}
