// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: MUSICFLS.CPP

#include <string.h>
#include <stdlib.h>
#include <alloc.h>
#include "AMUSIC.H"

// One entry of the global timbre library (GTL) header.
struct timbre_header
{
	char patch;
	char bank;
	long offset;
};

extern char far GTL_PREFIX[];

char *deleteMe = 0;
static char nim_file[] = "GENERIC.NIM";
static timbre_header gtl_header;
unsigned char Music::hdriver[2];

char Music::skip_blanks(BaseFile *file)
{
	char c;

	file->read(&c, 1);
	while (c == ' ' || c == '\t')
		file->read(&c, 1);
	return c;
}

void Music::skip_to_eol(BaseFile *file)
{
	char c;

	file->read(&c, 1);
	while (c != '\n')
		file->read(&c, 1);
}

int Music::read_to_white(BaseFile *file, char *buffer)
{
	char c;
	int count = 0;

	file->read(&c, 1);
	while (c != '\r' && c != '\n' && c != ' ' && c != '\t' && c != -1)
	{
		buffer[count] = c;
		file->read(&c, 1);
		count++;
	}
	buffer[count] = 0;
	return count;
}

// Music_halt calls pass the shipped __LINE__ values as literals.
int Music::load_timbres(file_info_type *files, int numFiles, unsigned char *form, unsigned char *)
{
	char *cache;
	char *timbre;
	drvr_desc *desc;
	int i;
	int numSequences;
	unsigned cacheSize;
	unsigned bank;
	unsigned patch;
	unsigned request;

	numSequences = form[0x14];
	desc = (drvr_desc *)XMI_describe_driver();
	char gtlName[32];
	strcpy(gtlName, GTL_PREFIX);
	strcat(gtlName, ".");
	strcat(gtlName, desc->data_suffix);
	if (mpu_401_device)
		return 1;
	cacheSize = XMI_default_timbre_cache_size();
	if (cacheSize)
	{
		deleteMe = cache = new char[cacheSize];
		XMI_define_timbre_cache(cache, cacheSize);
	}
	Music_Flex.getIndex(0x103, Music_Index);
	if (Music_Index.size == 0)
	{
		Music_halt(__FILE__, 195, 1, "Couldn't open GTL file.");
		return 0;
	}
	current_type seq;
	seq.st_ptr = new char[state_table_size];
	if (seq.st_ptr == 0)
	{
		Music_halt(__FILE__, 203, 2, "Couldn't allocate memory for state table in load_timbres.");
		return 0;
	}
	for (i = 0; i < numFiles; i++)
	{
		seq.handle = XMI_register_sequence(Activate_Music_File(files[i].tune), 0, seq.st_ptr, 0);
		if (seq.handle == -1)
		{
			farfree(seq.st_ptr);
			Music_halt(__FILE__, 214, 3, "An error has occurred in load_timbres.");
			return 0;
		}
		while ((request = XMI_timbre_request(seq.handle)) != 0xffff)
		{
			bank = request >> 8;
			patch = request & 0xff;
			timbre = load_global_timbre(bank, patch);
			if (timbre)
			{
				XMI_install_timbre(bank, patch, timbre);
				farfree(timbre);
			}
			else
				return -3;
		}
		XMI_release_sequence_handle(seq.handle);
	}
	for (i = 0; i < numSequences; i++)
	{
		seq.handle = XMI_register_sequence(form, i, seq.st_ptr, 0);
		if (seq.handle == -1)
		{
			farfree(seq.st_ptr);
			Music_halt(__FILE__, 264, 6, "Oops.  An error has occurred in load_timbres.");
			return 0;
		}
		while ((request = XMI_timbre_request(seq.handle)) != 0xffff)
		{
			bank = request >> 8;
			patch = request & 0xff;
			timbre = load_global_timbre(bank, patch);
			if (timbre)
			{
				XMI_install_timbre(bank, patch, timbre);
				farfree(timbre);
			}
			else
				return -5;
		}
		XMI_release_sequence_handle(seq.handle);
	}
	farfree(seq.st_ptr);
	return 1;
}

int Music::first_section(file_info_type *files)
{
	int count;
	char c;
	char skip;
	char word[80];
	char name[80];

	Music_Flex.seekRecord(0);
	count = 0;
	Music_Flex.read(&c, 1);
	while (c != '#')
	{
		if (c < '!')
			c = skip_blanks(&Music_Flex);
		read_to_white(&Music_Flex, word);
		name[0] = c;
		name[1] = 0;
		strcat(name, word);
		c = skip_blanks(&Music_Flex);
		Music_Flex.read(&skip, 1);
		read_to_white(&Music_Flex, word);
		if (key_array[c].used == 1)
		{
			Music_halt(__FILE__, 356, 8, "Error in .NIM file, to tunes assigned to same tune #");
			return 0;
		}
		files[count].data = 0;
		if (files[count].data)
			return -5;
		strcpy(word, "SOUND\\");
		strcat(word, name);
		strcpy(name, word);
		strcpy(files[count].name, name);
		files[count].tune = c;
		files[count].loop_measures = atol(word);
		read_to_white(&Music_Flex, word);
		files[count].restart_measure = atol(word);
		key_array[c].file = count;
		key_array[c].is_tune = 1;
		key_array[c].used = 1;
		Music_Flex.read(&c, 1);
		if (c < '!')
			c = skip_blanks(&Music_Flex);
		count++;
	}
	return count;
}

char *Music::second_section(void)
{
	char *buffer = 0;
	char c;
	long pos;
	char word[80];
	int record;

	skip_to_eol(&Music_Flex);
	read_to_white(&Music_Flex, word);
	Music_Flex.read(&c, 1);
	pos = Music_Flex.tell();
	record = 0x102;
	if (!mpu_401_device)
		record += 2;
	Music_Flex.getIndex(record, Music_Index);
	if (Music_Index.size == 0)
		Music_halt(__FILE__, 461, 9, "Couldn't load the transition file.");
	buffer = new char[Music_Index.size];
	Music_Flex.readRecord(record, buffer);
	Music_Flex.seek(pos);
	return buffer;
}

int Music::third_section(void)
{
	char c;

	skip_to_eol(&Music_Flex);
	Music_Flex.read(&c, 1);
	while (c != '#')
	{
		if (key_array[c].used == 1)
		{
			Music_halt(__FILE__, 513, 10, "Reassigning keystroke %c to an interrupt!!!  Aborting!");
			return 0;
		}
		key_array[c].used = 1;
		key_array[c].is_tune = 0;
		c = skip_blanks(&Music_Flex);
		if (c == '\n')
			Music_Flex.read(&c, 1);
	}
	return 1;
}

int Music::do_data_line(BaseFile *file, int line, int (*table)[100])
{
	char c;
	char number[8];
	int column;
	int i;
	int done;

	column = 1;
	if (table[line][0] != -1)
	{
		Music_halt(__FILE__, 569, 11, "line x of the transition table is already used! Aborting.");
		return 0;
	}
	table[line][0] = 1;
	c = skip_blanks(file);
	while (c != '\n')
	{
		if (c == ' ')
			c = skip_blanks(file);
		if (c == '\n')
			break;
		if (c == '!')
		{
			for (i = 0, done = 0; !done && i < 8; i++)
			{
				file->read(&c, 1);
				if (c == ' ' || c == '\n')
				{
					number[i] = 0;
					done = 1;
				}
				else
					number[i] = c;
			}
			table[line][column] = atol(number) + 5000;
		}
		else
		{
			number[0] = c;
			for (i = 1, done = 0; !done && i < 8; i++)
			{
				file->read(&c, 1);
				if (c == ' ' || c == '\n')
				{
					number[i] = 0;
					done = 1;
				}
				else
					number[i] = c;
			}
			table[line][column] = atol(number);
		}
		column++;
	}
	return 1;
}

int Music::fourth_section(file_info_type *files, int count)
{
	char c;
	int from;
	int line;
	char word[80];
	char fromName[80];
	char toName[80];
	int i;
	int to;
	int tune;

	line = 0;
	skip_to_eol(&Music_Flex);
	Music_Flex.read(&c, 1);
	while (c != '#')
	{
		read_to_white(&Music_Flex, word);
		fromName[0] = c;
		fromName[1] = 0;
		strcat(fromName, word);
		read_to_white(&Music_Flex, toName);
		i = 0;
		strcpy(word, "SOUND\\");
		strcat(word, fromName);
		strcpy(fromName, word);
		strcpy(word, "SOUND\\");
		strcat(word, toName);
		strcpy(toName, word);
		for (; strcmp(files[i].name, fromName) && i < count; i++)
			;
		from = i;
		for (i = 0; strcmp(files[i].name, toName) && i < count; i++)
			;
		to = i;
		tune = files[to].tune;
		if (from >= count || to >= count)
		{
			Music_halt(__FILE__, 703, 12, "Error! Couldn't find file");
			return 0;
		}
		table_table[from][tune] = line;
		do_data_line(&Music_Flex, line, trans_table);
		line++;
		Music_Flex.read(&c, 1);
		if (c < '!')
			c = skip_blanks(&Music_Flex);
	}
	return 1;
}

int Music::fifth_section(file_info_type *files, int count)
{
	char c;
	int line;
	char word[80];
	char name[80];
	int i;
	int found;
	int file;

	line = -1;
	for (i = 0, found = 0; i < 256 && !found; i++)
		if (trans_table[i][0] == -1)
		{
			line = i;
			found = 1;
		}
	if (line == -1)
	{
		Music_halt(__FILE__, 783, 13, "There's no room in trans_table!  Aborting.");
		return 0;
	}
	skip_to_eol(&Music_Flex);
	Music_Flex.read(&c, 1);
	while (c != '#')
	{
		read_to_white(&Music_Flex, word);
		name[0] = c;
		name[1] = 0;
		strcat(name, word);
		strcpy(word, "SOUND\\");
		strcat(word, name);
		strcpy(name, word);
		c = skip_blanks(&Music_Flex);
		for (i = 0; strcmp(files[i].name, name) && i < count; i++)
			;
		file = i;
		if (file >= count)
		{
			Music_halt(__FILE__, 813, 14, "Couldn't find a file");
			return 0;
		}
		if (table_table[file][c] != -1)
		{
			Music_halt(__FILE__, 821, 15, "Error in table_table.  Entry xx/xx is already used.");
			return 0;
		}
		table_table[file][c] = line;
		do_data_line(&Music_Flex, line, trans_table);
		line++;
		Music_Flex.read(&c, 1);
		if (c < '!')
			c = skip_blanks(&Music_Flex);
	}
	return 1;
}

char *Music::do_files(file_info_type *files, short *count, char *mode)
{
	unsigned char *percussion;
	char *transitions;

	*count = first_section(files);
	if (*count == 0)
	{
		Music_halt(__FILE__, 889, 17, "Couldn't find any files or error in first_section.");
		return 0;
	}
	transitions = second_section();
	if (!third_section())
	{
		Music_halt(__FILE__, 900, 19, "Error occurred in third_section.");
		return 0;
	}
	if (!fourth_section(files, *count))
	{
		Music_halt(__FILE__, 908, 20, "Error occurred in fourth_section.");
		return 0;
	}
	if (!fifth_section(files, *count))
	{
		Music_halt(__FILE__, 917, 21, "Error occurred in fifth_section.");
		return 0;
	}
	if (*mode == 'm' || *mode == 'M')
	{
		if (!load_timbres(files, *count, (unsigned char *)transitions, percussion))
		{
			Music_halt(__FILE__, 945, 23, "Error in load_timbres.  Aborting.");
			return 0;
		}
		current_type a_seq;
		a_seq.st_ptr = new char[state_table_size];
		if (a_seq.st_ptr == 0)
		{
			Music_halt(__FILE__, 953, 24, "Couldn't allocate memory for a_seq.st_ptr!");
			return 0;
		}
		a_seq.handle = XMI_register_sequence(percussion, 0, a_seq.st_ptr, 0);
		if (a_seq.handle == -1)
		{
			delete a_seq.st_ptr;
			Music_halt(__FILE__, 961, 25, "Encountered a problem playing percussy.xmi.");
			return 0;
		}
		XMI_start_sequence(a_seq.handle);
		while (XMI_sequence_status(a_seq.handle) != 2)
			;
		XMI_release_sequence_handle(a_seq.handle);
		delete a_seq.st_ptr;
	}
	else if (!load_timbres(files, *count, (unsigned char *)transitions, 0))
	{
		Music_halt(__FILE__, 978, 26, "Error in load_timbres.  Aborting.");
		return 0;
	}
	return transitions;
}

char *Music::load_global_timbre(unsigned bank, unsigned patch)
{
	char *timbre;
	int length;

	Music_Flex.getIndex(0x103, Music_Index);
	if (Music_Index.size == 0)
		return 0;
	Music_Flex.seekRecord(0x103);
	do
	{
		Music_Flex.read((char *)&gtl_header, 6);
		if (gtl_header.bank == -1)
			return 0;
	} while (gtl_header.bank != bank || gtl_header.patch != patch);
	Music_Flex.seekRecord(0x103);
	Music_Flex.seek(gtl_header.offset, FromCurrent);
	Music_Flex.read((char *)&length, 2);
	if (length == 0)
		return 0;
	timbre = (char *)farmalloc(length + 4);
	*(int *)timbre = length;
	Music_Flex.read(timbre + 2, length - 2);
	return timbre;
}
