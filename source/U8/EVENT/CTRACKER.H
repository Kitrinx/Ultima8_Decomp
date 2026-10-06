#ifndef CTRACKER_H
#define CTRACKER_H

#include "CMOUSE.H"

// Hands mouse clicks to the debug tracker, and remembers the last few
// source places reported through SetArea.
class Tracker : public MouseHandler
{
public:
	static int head;
	static char *area_file[10];
	static int area_line[10];

	Tracker(void);
	static void set_area(char *, int);
	void report_area(void);
	virtual void handle(int, int, int, int);
};

void SetArea(char *, int);

#endif
