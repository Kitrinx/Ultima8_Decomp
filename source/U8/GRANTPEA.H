#ifndef GRANTPEA_H
#define GRANTPEA_H

#include "..\GLIB\PROCESS.H"

class GrantPeaceSpell : public Process
{
public:
	GrantPeaceSpell(void);
	virtual void process(void);
};

void createNpc(unsigned short);
void castGrantPeaceSpell(void);

#endif
