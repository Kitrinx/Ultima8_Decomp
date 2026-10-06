// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: ..\ITEM\SCRIT.C

#include <stdarg.h>
#include <mem.h>
#include "ERROR.H"
#include "SCRIT.H"

SearchCriteria::SearchCriteria(short command, ...)
{
	int pos = 1;
	int c;
	int value;
	int i;
	int len;
	va_list ap;

	va_start(ap, command);
	for (c = command; c != '$'; c = va_arg(ap, int))
	{
		len = cmdlen(c);
		if (len + pos < 31)
		{
			data[pos++] = c;
			for (i = 1; i < len; i += 2)
			{
				value = va_arg(ap, int);
				data[pos++] = value & 0xff;
				data[pos++] = value >> 8;
			}
		}
		else
			halt(__FILE__, 66);	// __LINE__
	}
	data[pos] = '$';
	data[0] = pos;
}

int SearchCriteria::cmdlen(short c)
{
	if (c == '%')
		return 3;
	if (c > '@' && c < '`')
		return (c - '@') * 2 + 1;
	if (c > '`' && c < 0x80)
		return (c - '`') * 2 + 1;
	return 1;
}

void SearchCriteria::operator+=(SearchCriteria other)
{
	int len = data[0];
	int otherLen = other.data[0];
	data[0] = len + otherLen - 1;
	memcpy(&data[len], &other.data[1], otherLen);
}
