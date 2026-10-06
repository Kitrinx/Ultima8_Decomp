// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: GRAPHICS\MERGEARY.C

#include "CDESC.H"

// Fades dest toward src one step per call, Bresenham style.
int init = 1;
char *error;
char *delta;
char *vector;
int major;
int iterations;

int pascal MERGEBYTEARRAY(char *dest, char *src, short count)
{
	int i;
	int d;

	if (init) {
		error = new char[count];
		delta = new char[count];
		vector = new char[count];
		if (error == 0 || delta == 0 || vector == 0) {
			if (error)
				delete error;
			if (delta)
				delete delta;
			if (vector)
				delete vector;
			return 0;
		}
		major = 0;
		for (i = 0; i < count; i++) {
			d = dest[i] - src[i];
			if (d < 0) {
				d = -d;
				vector[i] = 1;
			} else
				vector[i] = -1;
			delta[i] = d;
			if (d > major)
				major = d;
		}
		d = major / 2;
		for (i = 0; i < count; i++)
			error[i] = d;
		iterations = major;
		init = 0;
	}
	if (iterations-- == 0) {
		delete error;
		delete delta;
		delete vector;
		init = 1;
		return 0;
	}
	for (i = 0; i < count; i++) {
		if ((error[i] += delta[i]) > major) {
			error[i] -= major;
			dest[i] += vector[i];
		}
	}
	return 1;
}
