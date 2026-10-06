#ifndef CCURRVP_H
#define CCURRVP_H

#include "CGLOBVP.H"

// Makes a viewport the global one until it goes out of scope.
class CurrentVport : public GlobalVport
{
public:
	Vport *saved;

	CurrentVport(Vport *);
	~CurrentVport(void);
};

#endif
