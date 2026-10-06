#ifndef RANDINT_H
#define RANDINT_H

extern "C" int RandInt(void);
extern "C" int Ultima5Random(int, int);
extern "C" void SetRandomSeed(long);
extern "C" long GetRandomSeed(void);
extern "C" int PseudoRandom(int);

#endif
