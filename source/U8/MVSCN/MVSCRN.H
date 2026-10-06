#ifndef MVSCRN_H
#define MVSCRN_H

class Vport;

// Scroll a viewport's contents by the given number of pixels.
extern "C" void MoveScreenUp(Vport *, int);
extern "C" void MoveScreenDown(Vport *, int);
extern "C" void MoveScreenLeft(Vport *, int);
extern "C" void MoveScreenRight(Vport *, int);
extern "C" void MoveScreenUpLeft(Vport *, int, int);
extern "C" void MoveScreenUpRight(Vport *, int, int);
extern "C" void MoveScreenDownLeft(Vport *, int, int);
extern "C" void MoveScreenDownRight(Vport *, int, int);

void MoveScreen(Vport *, short, short);

#endif
