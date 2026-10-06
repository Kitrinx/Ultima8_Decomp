#ifndef URANDOM_H
#define URANDOM_H

#include "RANDINT.H"

int urandom(int);
void urandomize(void);
int rndRange(short, short);
unsigned char OneIn(short);

#endif
