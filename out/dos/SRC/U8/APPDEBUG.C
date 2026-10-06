// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d -r-
// name: APPDEBUG.C

#include <stdio.h>
#include "NPC.H"
#include "INIT.H"
#include "UPROCESS.H"
#include "DSFXMAN.H"
#include "APPDEBUG.H"

void standardDebug(char *, int)
{
}

void initDebug(void)
{
}

void applicationGarbageCollect(unsigned char flush)
{
	while (RoutineIndex::cacheOut(-1))
		;
	checkFlush(flush ? -1 : 0);
}

void applicationOutOfMemory(unsigned short size)
{
	static char line[] = "-----------------------------------------------------------\r\n";
	static char english[] = "Ultima 8 has an insufficient amount of dynamic memory left to\r\n"
		"continue. You may increase the amount by typing the following:\r\n"
		"\tu8 -d%d\r\n\r\n\r\n";
	static char french[] = "Ultima 8 a un montant insuffisant de m\x82moire dynamique pour\r\n"
		"continuer. Vous pouvez augmenter ce montant en tapant:\r\n"
		"\tu8 -d%d\r\n\r\n\r\n";
	static char german[] = "Ultima 8 verf\x81gt nicht \x81" "ber ausreichende dynamische\r\n"
		"Speicherkapazit\x84t. Sie k\x94nnen Ihre Speicherkapazit\x84t\r\n"
		"erh\x94hen, indem Sie u8 -d%d tippen.\r\n\r\n\r\n";

	size += 5;
	printf(line);
	printf("\r\n");
	switch (avatar.language)
	{
	case ENGLISH:
		printf(english, size);
		break;
	case FRENCH:
		printf(french, size);
		break;
	case GERMAN:
		printf(german, size);
		break;
	default:
		printf(english, size);
		printf(french, size);
		printf(german, size);
		break;
	}
	printf(line);
}
