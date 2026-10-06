#ifndef CDLIST_H
#define CDLIST_H

class DoubleLink
{
public:
	DoubleLink *next;
	DoubleLink *prev;

	DoubleLink(DoubleLink *, DoubleLink *);
	virtual ~DoubleLink(void);
};

class DoubleLinkedList
{
public:
	DoubleLink *head;
	DoubleLink *tail;

	DoubleLinkedList(void);
	virtual ~DoubleLinkedList(void);
	void addToHead(DoubleLink *);
	void addToTail(DoubleLink *);
	void insertAfter(DoubleLink *, DoubleLink *);
	void insertBefore(DoubleLink *, DoubleLink *);
	void moveAfter(DoubleLink *, DoubleLink *);
	void moveBefore(DoubleLink *, DoubleLink *);
	void moveToHead(DoubleLink *);
	void moveToTail(DoubleLink *);
	void remove(DoubleLink *);
	void destroy(DoubleLink *);
	void kill(void);
	int traverse(DoubleLink *&);
	int traverseBackward(DoubleLink *&);
	long count(void);
};

class Link
{
public:
	Link *next;

	Link(void) { next = 0; }
	virtual ~Link(void);
};

#endif
