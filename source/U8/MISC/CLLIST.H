#ifndef CLLIST_H
#define CLLIST_H

class Link;

class LinkedList
{
public:
	Link *head;
	Link *tail;

	LinkedList(void);
	virtual ~LinkedList(void);
	void addToHead(Link *);
	void addToTail(Link *);
	void destroy(Link *, Link *);
	void insertAfter(Link *, Link *);
	void remove(Link *, Link *);
	void kill(void);
	int traverse(Link *&);
	long count(void);
};

#endif
