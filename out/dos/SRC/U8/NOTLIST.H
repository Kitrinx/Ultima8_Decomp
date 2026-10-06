#ifndef NOTLIST_H
#define NOTLIST_H

#include "MLIST.H"

class NotifyList : public List
{
public:
	void notify(unsigned short);
};

#endif
