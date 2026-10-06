// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: URANDOM.C

#include "STATREST.H"

// 0 to n - 1.
int urandom(int n)
{
	return PseudoRandom(n);
}

void urandomize(void)
{
	SetRandomSeed(0x12345678L);
}

// low to high inclusive, either way round.
int rndRange(short low, short high)
{
	if (low <= high)
		return urandom(high - low + 1) + low;
	return urandom(low - high + 1) + high;
}

// True one time in n.
unsigned char OneIn(short n)
{
	return urandom(n) == 0;
}
