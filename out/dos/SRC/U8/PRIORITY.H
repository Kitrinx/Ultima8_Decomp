#ifndef PRIORITY_H
#define PRIORITY_H

// A process priority test: a level in the low 5 bits and which of
// less (0x40), equal (0x20) and greater (0x80) match in the high 3.
class PriorityClass
{
public:
	unsigned char valid;

	PriorityClass(unsigned char v) { valid = v; }
	unsigned char isValid(void);
	unsigned char contains(unsigned char);
};

#endif
