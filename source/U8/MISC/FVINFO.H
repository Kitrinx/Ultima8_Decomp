#ifndef FVINFO_H
#define FVINFO_H

class Point;
class Vport;

extern "C" int GetMaxShapeWidth(void *);
extern "C" int GetMaxShapeDepth(void *);
extern "C" int GetNumShapes(void *);
extern "C" void *GetShapeAddress(void *, int);
extern "C" int SkipDraw(Vport *, int, int, void *);
extern "C" int SkipTransformDraw(Vport *, int, int, void *, void *);
void GetShapeInfo(char *, short, short &, short &, short &, short &);
extern "C" void TransformViewport(Vport);
extern "C" void Scale(void *);
unsigned char figureDiamond(Point, Point, Point &, Point &, Point &, Point &);
void DrawDiamond(Vport *, short, short, short, short, unsigned char);
void DrawFilledDiamond(Vport *, short, short, short, short, unsigned char);

#endif
