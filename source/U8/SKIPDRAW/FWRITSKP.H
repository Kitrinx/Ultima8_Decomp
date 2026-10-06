#ifndef FWRITSKP_H
#define FWRITSKP_H

class Vport;

unsigned EstimateFastWriteSkip(Vport &, short, short, short, short, short);
int FastWriteSkip(Vport &, short, short, short, short, short, short, unsigned char *, short);

#endif
