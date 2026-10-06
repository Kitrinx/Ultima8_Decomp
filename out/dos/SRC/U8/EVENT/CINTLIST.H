#ifndef CINTLIST_H
#define CINTLIST_H

#include "CDLIST.H"
#include "CEXIT.H"

class InterruptList;

// One handler hooked onto an interrupt vector. Several links may share a
// vector; each chains to the handler installed before it.
class IntrLink : public DoubleLink
{
public:
	InterruptList *list;
	void (*oldHandler)(...);
	short vector;
	void (**chain)(...);	// the owner's copy of oldHandler, kept current

	IntrLink(void);
	virtual ~IntrLink(void);
	void set_vector(void (*)(...));
	void attach(InterruptList *);
};

class InterruptList : public DoubleLinkedList, public ExitSystem
{
public:
	virtual ~InterruptList(void);
	virtual void exit_code(void);
	void private_remove_interrupt(IntrLink *);
	void install_interrupt(short, void (*)(...), IntrLink *, void (**)(...));
	void install_interrupt_before(IntrLink *, void (*)(...), IntrLink *, void (**)(...));
	void remove_interrupt(short);
	void remove_interrupt(IntrLink *);
	void remove_all_interrupts(void);
};

extern InterruptList TheInterruptList;

#endif
