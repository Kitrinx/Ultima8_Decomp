// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MISC\CDLIST.C

#include "CDLIST.H"

DoubleLink::~DoubleLink(void)
{
}

void DoubleLinkedList::addToHead(DoubleLink *link)
{
	link->prev = 0;
	link->next = head;
	if (head)
		head->prev = link;
	else
		tail = link;
	head = link;
}

void DoubleLinkedList::addToTail(DoubleLink *link)
{
	link->prev = tail;
	link->next = 0;
	if (tail)
		tail->next = link;
	else
		head = link;
	tail = link;
}

void DoubleLinkedList::insertAfter(DoubleLink *position, DoubleLink *link)
{
	if (link) {
		if (!position) {
			addToHead(link);
			return;
		}
		if (tail != position) {
			link->next = position->next;
			link->prev = position;
			position->next->prev = link;
			position->next = link;
			return;
		}
		tail = link;
		link->next = 0;
		link->prev = position;
		position->next = link;
	}
}

void DoubleLinkedList::insertBefore(DoubleLink *position, DoubleLink *link)
{
	if (link) {
		if (!position) {
			addToTail(link);
			return;
		}
		if (head != position) {
			link->next = position;
			link->prev = position->prev;
			position->prev->next = link;
			position->prev = link;
			return;
		}
		head = link;
		link->prev = 0;
		link->next = position;
		position->prev = link;
	}
}

void DoubleLinkedList::moveAfter(DoubleLink *position, DoubleLink *link)
{
	remove(link);
	insertAfter(position, link);
}

void DoubleLinkedList::moveBefore(DoubleLink *position, DoubleLink *link)
{
	remove(link);
	insertBefore(position, link);
}

void DoubleLinkedList::moveToHead(DoubleLink *link)
{
	remove(link);
	insertAfter(0, link);
}

void DoubleLinkedList::moveToTail(DoubleLink *link)
{
	remove(link);
	insertBefore(0, link);
}

void DoubleLinkedList::remove(DoubleLink *link)
{
	DoubleLink *prev;

	if (link) {
		prev = link->prev;
		if (prev)
			prev->next = link->next;
		else
			head = link->next;
		if (link->next) {
			link->next->prev = prev;
			return;
		}
		if (prev)
			prev->next = 0;
		else
			head = 0;
		tail = prev;
	}
}

void DoubleLinkedList::destroy(DoubleLink *link)
{
	remove(link);
	delete link;
}

void DoubleLinkedList::kill(void)
{
	DoubleLink *next;

	while (head) {
		next = head->next;
		delete head;
		head = next;
	}
	head = tail = 0;
}

int DoubleLinkedList::traverse(DoubleLink *&link)
{
	if (!link)
		link = head;
	else
		link = link->next;
	return link != 0;
}

int DoubleLinkedList::traverseBackward(DoubleLink *&link)
{
	if (!link)
		link = tail;
	else
		link = link->prev;
	return link != 0;
}

long DoubleLinkedList::count(void)
{
	long n = 0;
	DoubleLink *link;

	for (link = head; link; link = link->next)
		n++;
	return n;
}
