#ifndef CYTABLE_H
#define CYTABLE_H

// Row start offsets of a screen buffer, `step` bytes apart.
class GraphicYtable
{
public:
	unsigned short *index;
	unsigned char f_04;

	GraphicYtable(void);
	void setType(unsigned char);
	unsigned char verify_ytable(short, short);
	unsigned char init_ytable(short, short, unsigned short);
	int getindex(short);
	void setindex(short y, unsigned short offset) { index[y] = offset; }
	void free(void)
	{
		if (index) {
			delete index;
			index = 0;
		}
	}
};

#endif
