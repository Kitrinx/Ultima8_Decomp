// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MISC\CLLIST.C

#include "CDLIST.H"
#include "CLLIST.H"

Link::~Link(void)
{
}

void LinkedList::addToHead(Link *link)
{
	if (link) {
		link->next = head;
		head = link;
		if (!tail)
			tail = head;
	}
}

// Leaves link->next alone: callers pass a link that ends its chain.
void LinkedList::addToTail(Link *link)
{
	if (link) {
		if (tail)
			tail->next = link;
		else
			head = link;
		tail = link;
	}
}

void LinkedList::destroy(Link *link, Link *prev)
{
	remove(link, prev);
	delete link;
}

// Inserts the whole chain starting at link.
void LinkedList::insertAfter(Link *position, Link *link)
{
	Link *last;
	Link *p;

	if (!position)
		return;
	if (!link)
		return;
	last = link;
	for (p = link->next; p; p = p->next)
		last = p;
	if (tail != position) {
		last->next = position->next;
		position->next = link;
		return;
	}
	tail = last;
	last->next = 0;
	position->next = link;
}

// prev is the link before link, or 0 to search for it.
void LinkedList::remove(Link *link, Link *prev)
{
	Link *p;

	if (!prev) {
		for (p = head; p && p != link; p = p->next)
			prev = p;
		if (!p)
			return;
	}
	if (!prev)
		head = head->next;
	if (tail != link) {
		if (prev)
			prev->next = link->next;
		if (head == link)
			head = link->next;
	} else {
		tail = prev;
		if (prev)
			prev->next = 0;
	}
}

void LinkedList::kill(void)
{
	Link *next;

	while (head) {
		next = head->next;
		delete head;
		head = next;
	}
	head = tail = 0;
}

int LinkedList::traverse(Link *&link)
{
	if (!head && !link)
		return 0;
	if (!link)
		link = head;
	else
		link = link->next;
	return link != 0;
}

long LinkedList::count(void)
{
	long n = 0;
	Link *link;

	for (link = head; link; link = link->next)
		n++;
	return n;
}
