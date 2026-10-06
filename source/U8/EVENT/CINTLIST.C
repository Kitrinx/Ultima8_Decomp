// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: EVENT\CINTLIST.C

#include <dos.h>
#include "CINTLIST.H"

typedef void interrupt (*Vector)(...);

// Inline in the shared headers when this file was built.
inline DoubleLink::DoubleLink(DoubleLink *n, DoubleLink *p) { next = n; prev = p; }
inline DoubleLinkedList::DoubleLinkedList(void) { head = 0; tail = 0; }
inline DoubleLinkedList::~DoubleLinkedList(void) { kill(); }
inline void IntrLink::attach(InterruptList *l) { list = l; }
inline InterruptList::~InterruptList(void) { remove_all_interrupts(); }

InterruptList TheInterruptList;

IntrLink::IntrLink(void) :
	DoubleLink(0, 0)
{
	vector = -1;
	oldHandler = 0;
	chain = 0;
	list = 0;
}

IntrLink::~IntrLink(void)
{
	if (list)
		list->remove_interrupt(this);
}

void IntrLink::set_vector(void (*handler)(...))
{
	oldHandler = handler;
	if (chain)
		*chain = handler;
}

// Hands link's vector back to whatever it displaced.
void InterruptList::private_remove_interrupt(IntrLink *link)
{
	DoubleLink *prev = link;

	while (traverseBackward(prev))
		if (((IntrLink *)prev)->vector == link->vector)
			break;
	if (prev)
		((IntrLink *)prev)->set_vector(link->oldHandler);
	else
		setvect(link->vector, (Vector)link->oldHandler);
	link->set_vector(0);
	link->vector = -1;
	remove(link);
	link->list = 0;
}

void InterruptList::install_interrupt(short vector, void (*handler)(...), IntrLink *link, void (**chain)(...))
{
	link->vector = vector;
	link->chain = chain;
	link->set_vector((void (*)(...))getvect(vector));
	setvect(vector, (Vector)handler);
	addToHead(link);
	link->attach(this);
}

// Puts link's handler in the chain just ahead of position's.
void InterruptList::install_interrupt_before(IntrLink *position, void (*handler)(...), IntrLink *link, void (**chain)(...))
{
	link->vector = position->vector;
	link->chain = chain;
	link->set_vector(position->oldHandler);
	disable();
	position->set_vector(handler);
	enable();
	insertAfter(position, link);
}

void InterruptList::remove_interrupt(short vector)
{
	DoubleLink *link = head;

	do
	{
		if (((IntrLink *)link)->vector == vector)
			break;
	} while (traverse(link));
	if (link)
		private_remove_interrupt((IntrLink *)link);
}

void InterruptList::remove_interrupt(IntrLink *link)
{
	DoubleLink *p;

	if (link)
	{
		p = head;
		while (p != link && traverse(p))
			;
		if (p)
			private_remove_interrupt(link);
	}
}

void InterruptList::remove_all_interrupts(void)
{
	DoubleLink *link = head;

	do
	{
		if (link)
			private_remove_interrupt((IntrLink *)link);
	} while (traverse(link));
}

void InterruptList::exit_code(void)
{
	remove_all_interrupts();
}
