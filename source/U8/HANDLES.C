// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: HANDLES.C

#include <io.h>
#include <fcntl.h>
#include "APPDEBUG.H"

// Counts free DOS file handles by opening one file until it fails.
int howManyFreeHandles(void)
{
	int i, count = 0;
	int handles[100];

	for (i = 0; i < 100; i++)
	{
		handles[i] = open("\\autoexec.bat", O_BINARY);
		if (handles[i] == -1)
		{
			count = i;
			break;
		}
	}
	for (i--; i >= 0; i--)
		close(handles[i]);
	return count;
}
