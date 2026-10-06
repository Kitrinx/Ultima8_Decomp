// flags: -P -3 -O -Ob -Oe -Og -Oi -Ol -Om -Ov -Z -G -k- -d
// name: AMUSIC.CPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include "AMUSIC.H"
#include "FEXIST.H"
#include "MUSICFLS.H"

// Inline in the shared headers when this file was built.
inline void nameProcess(unsigned pid, char *name) { ((char **)Kernel::idStringList)[pid] = name; }
inline unsigned char BaseFile::is_valid(void) { return handle != -1; }
inline BaseFile::~BaseFile(void) { if (is_valid()) close(); }
inline SharedFile::SharedFile(void) {}
inline FlexFile::FlexFile(void) {}
inline FastFlex::FastFlex(void) {}
inline Index::Index(void) { size = 0; offset = 0; }

void *TheMusicProcess = 0;
int Music::speedhack = 1;
int Music::music_volume = 100;
char *Music::Music_Buffer[2] = {0, 0};
Index *Music::Music_Index_Ptr = 0;
int Music::last_read[2] = {0, 0};
long Music::state_table_size;
int Music::old_measure;
int Music::new_measure;
int (*Music::trans_table)[100];
int (*Music::table_table)[256];
int Music::desired_tune;
int Music::current_tune;
int Music::initialized;
int Music::hooked;
short Music::num_ms;
int Music::music_status;
int Music::ref_meas;
int Music::music_play_state;
int Music::music_stop_state;
int Music::slow_stop_state;
int Music::mpu_401_device;
int Music::auto_detect;
int Music::music_dma;
int Music::music_irq;
int Music::music_drq;
int Music::music_port;
char Music::music_branch[256];
FastFlex Music::Music_Flex;
Index Music::Music_Index;
char *Music::trans_file_ptr;
int Music::play_queue;
file_info_type *Music::ms_files;
key_type *Music::key_array;
current_type Music::current_ms;
current_type Music::current_other;
current_type *Music::current_temp;
current_type Music::a_seq;

// Music_halt callers pass the shipped __LINE__ values as literals.
void Music::Music_halt(char *file, int line, int code, char *message)
{
	deinit();
	printf("\nError #: %d (Music Manager)\nError Message: %s\n", code, message);
	printf("%s, %d\n", file, line);
	exit(0);
}

int Music::init(char *name, int port, int irq, int dma, int drq)
{
	char id[10];
	char path[128];

	strcpy(path, "SOUND\\");
	strcat(path, name);
	Music_Flex.init(path, Music_Index_Ptr, ReadOnly, -1);
	trans_table = new int[300][100];
	table_table = new int[75][256];
	ms_files = new file_info_type[75];
	key_array = new key_type[257];
	if (irq == 0 || dma == 0)
		auto_detect = 1;
	else
	{
		music_dma = dma;
		music_irq = irq;
		music_drq = drq;
		music_port = port;
	}
	last_read[0] = last_read[1] = 0;
	initialized = 0;
	old_measure = 0;
	new_measure = 0;
	num_ms = 0;
	slow_stop_state = 0;
	music_status = 0;
	current_tune = -2;
	desired_tune = -1;
	hooked = 0;
	music_stop_state = 0;
	mpu_401_device = 0;
	music_play_state = 1;
	play_queue = -1;
	trans_file_ptr = 0;
	BaseFile file;
	current_ms.st_ptr = 0;
	current_other.st_ptr = 0;
	for (int i = 0; i < 255; i++)
		music_branch[i] = -1;
	file.open("AILXMI.DLL", ReadWrite);
	if (file.seek(0x315d))
	{
		file.read(id, 3);
		id[3] = 0;
		if (!strcmp(id, "MPU"))
			mpu_401_device = 1;
	}
	file.close();
	char driver[40];
	strcpy(driver, "AILXMI.DLL");
	if (!FileExists(path))
		return -2;
	Music_Flex.getIndex(0, Music_Index);
	if (Music_Index.size == 0)
		return -2;
	if (!init_AIL(&state_table_size))
		return -2;
	trans_file_ptr = do_files(ms_files, &num_ms, driver);
	if (trans_file_ptr == 0)
	{
		Music_halt(__FILE__, 206, -2, "Error in do_files");
		return -2;
	}
	current_ms.st_ptr = new char[state_table_size];
	if (current_ms.st_ptr == 0)
	{
		Music_halt(__FILE__, 217, -3, "Error, couldn't allocate memory for the current_ms state table");
		return -2;
	}
	current_other.st_ptr = new char[state_table_size];
	if (current_other.st_ptr == 0)
	{
		Music_halt(__FILE__, 225, -4, "Error, couldn't allocate memory for the current_other state table");
		return -2;
	}
	current_ms.active = 0;
	current_other.active = 0;
	current_ms.measure = 1;
	current_ms.restart = 1;
	music_hook();
	initialized = 1;
	return 0;
}

int Music::play(int tune)
{
	static int result = 0;
	int wanted;

	if (!initialized)
		return 0;
	if (music_status)
	{
		play_queue = tune;
		return -123;
	}
	if (tune == current_tune)
	{
		if (slow_stop_state)
		{
			stop();
			slow_stop_state = 0;
		}
		else
			return 0;
	}
	wanted = tune;
	if (current_tune < 0)
		stop();
	if (current_tune > 0)
	{
		if (!music_play_state)
		{
			result = -15;
			return result;
		}
		if (slow_stop_state)
		{
			stop();
			slow_stop_state = 0;
		}
	}
	if (current_tune < -1)
		current_tune = tune;
	desired_tune = tune;
	result = update_music();
	if (result < -8)
	{
		stop();
		current_tune = desired_tune = wanted;
		result = update_music();
	}
	if (result >= 0)
	{
		music_stop_state = 1;
		result = current_tune = desired_tune;
	}
	return result;
}

int Music::resume(void)
{
	if (!initialized)
		return 0;
	if (current_ms.active == 1)
	{
		XMI_resume_sequence(current_ms.handle);
		music_play_state = 1;
		if (current_other.active == 1 && current_other.handle >= 0)
			XMI_resume_sequence(current_other.handle);
	}
	else if (current_other.active == 1 && current_other.handle >= 0)
	{
		XMI_resume_sequence(current_other.handle);
		music_play_state = 1;
	}
	return 1;
}

int Music::pause(void)
{
	if (!initialized)
		return 0;
	if (current_ms.active == 1 && current_ms.handle >= 0)
	{
		XMI_stop_sequence(current_ms.handle);
		music_play_state = 0;
		if (current_other.active == 1 && current_other.handle >= 0)
			XMI_stop_sequence(current_other.handle);
	}
	else if (current_other.active == 1 && current_other.handle >= 0)
	{
		XMI_stop_sequence(current_other.handle);
		music_play_state = 0;
	}
	else
		return 0;
	return 1;
}

void Music::stop(void)
{
	int wasStatus;

	if (!initialized)
		return;
	if (current_tune < 0)
		return;
	wasStatus = music_status;
	music_status = 0;
	music_stop_state = 0;
	current_tune = -2;
	desired_tune = -1;
	new_measure = old_measure = 0;
	current_ms.measure = 1;
	if (wasStatus)
	{
		if (current_ms.active == 1 && current_ms.handle >= 0)
			XMI_stop_sequence(current_ms.handle);
		if (current_other.active == 1 && current_other.handle >= 0)
			XMI_stop_sequence(current_other.handle);
		if (current_ms.handle >= 0)
		{
			XMI_release_sequence_handle(current_ms.handle);
			current_ms.handle = -1;
		}
		if (current_other.handle >= 0)
		{
			XMI_release_sequence_handle(current_other.handle);
			current_other.handle = -1;
		}
		if (a_seq.handle >= 0)
		{
			XMI_release_sequence_handle(a_seq.handle);
			a_seq.handle = -1;
		}
		if (a_seq.st_ptr)
			delete a_seq.st_ptr;
		a_seq.st_ptr = 0;
		current_ms.active = 0;
		current_other.active = 0;
		music_status = 0;
		return;
	}
	if (current_ms.active == 1 && current_ms.handle >= 0)
	{
		XMI_stop_sequence(current_ms.handle);
		XMI_release_sequence_handle(current_ms.handle);
		current_ms.active = 0;
		current_ms.handle = -1;
	}
	if (current_other.active == 1 && current_other.handle >= 0)
	{
		XMI_stop_sequence(current_other.handle);
		XMI_release_sequence_handle(current_other.handle);
		current_other.active = 0;
		current_other.handle = -1;
	}
}

int Music::xwait_for_eom(current_type *seq)
{
	static int measure;

	if (seq->active != 1 || seq->handle < 0)
		return 1234;
	measure = XMI_measure_count(seq->handle);
	if (measure < 1)
		measure = 1;
	if (ref_meas == measure && XMI_sequence_status(seq->handle) == 1)
		return -1;
	return ref_meas != measure ? 1 : 2;
}

int Music::end_measure(current_type seq)
{
	new_measure = XMI_measure_count(seq.handle);
	if (old_measure == new_measure)
		return 0;
	old_measure = new_measure;
	return 1;
}

int Music::do_normal(int key)
{
	char *form;
	int line;
	int next;

	if (key_array[key].is_tune == 0)
	{
		line = table_table[current_ms.file][key];
		if (line == -1)
			return -3;
		next = trans_table[line][current_ms.measure];
		if (next == 0)
			return -4;
		if (next > 5000)
		{
			current_other.handle = XMI_register_sequence(trans_file_ptr, next - 5001, current_other.st_ptr, 0);
			if (current_other.handle == -1)
				return -5;
			music_status = 8;
			ref_meas = XMI_measure_count(current_ms.handle);
		}
		else
		{
			current_other.handle = XMI_register_sequence(trans_file_ptr, next - 1, current_other.st_ptr, 0);
			if (current_other.handle == -1)
				return -6;
			ref_meas = XMI_measure_count(current_ms.handle);
			music_status = 4;
		}
	}
	else
	{
		line = table_table[current_ms.file][key];
		if (line != -1)
			next = trans_table[line][current_ms.measure];
		else
		{
			next = 0;
			stop();
			current_tune = desired_tune = key;
		}
		if (next == 0)
		{
			a_seq.st_ptr = new char[state_table_size];
			if (a_seq.st_ptr == 0)
				return -8;
			a_seq.file = key_array[key].file;
			form = Activate_Music_File(ms_files[key_array[key].file].tune);
			if (form == 0)
				return -9;
			a_seq.handle = XMI_register_sequence(form, 0, a_seq.st_ptr, 0);
			if (a_seq.handle == -1)
				return -9;
			current_tune = desired_tune;
			music_status = 3;
			if (current_ms.active == 1 && current_ms.handle >= 0)
				ref_meas = XMI_measure_count(current_ms.handle);
			return 0;
		}
		if (next > 5000)
		{
			current_other.handle = XMI_register_sequence(trans_file_ptr, next - 5001, current_other.st_ptr, 0);
			if (current_other.handle == -1)
				return -10;
			a_seq.st_ptr = new char[state_table_size];
			if (a_seq.st_ptr == 0)
				return -11;
			form = Activate_Music_File(ms_files[key_array[key].file].tune);
			if (form == 0)
				return -12;
			a_seq.handle = XMI_register_sequence(form, 0, a_seq.st_ptr, 0);
			if (a_seq.handle == -1)
				return -12;
			current_ms.active = 1;
			ref_meas = XMI_measure_count(current_ms.handle);
			music_status = 6;
		}
		else
		{
			current_other.handle = XMI_register_sequence(trans_file_ptr, next - 1, current_other.st_ptr, 0);
			if (current_other.handle == -1)
				return -13;
			ref_meas = XMI_measure_count(current_ms.handle);
			music_status = 1;
		}
	}
	return 0;
}

int Music::init_AIL(long *tableSize)
{
	drvr_desc *desc;
	int i;
	int j;

	desc = (drvr_desc *)XMI_describe_driver();
	if (desc->drvr_type != 4)
	{
		Music_halt(__FILE__, 851, 28, "Unable to load XMIDI driver");
		return 0;
	}
	if (auto_detect)
	{
		if (!XMI_detect_device(desc->default_IO, desc->default_IRQ, desc->default_DMA, desc->default_DRQ))
			return 0;
	}
	else
	{
		desc->default_IO = music_port;
		desc->default_IRQ = music_irq;
		desc->default_DMA = music_dma;
		desc->default_DRQ = music_drq;
	}
	XMI_init_driver(desc->default_IO, desc->default_IRQ, desc->default_DMA, desc->default_DRQ);
	*tableSize = XMI_state_table_size();
	for (i = 0; i < 256; i++)
		key_array[i].used = 0;
	for (i = 0; i < 75; i++)
		for (j = 0; j < 256; j++)
			table_table[i][j] = -1;
	for (i = 0; i < 300; i++)
		for (j = 0; j < 100; j++)
			trans_table[i][j] = -1;
	return 1;
}

MusicProcess::MusicProcess(short hertz)
{
	setHertz(hertz);
	intFlags = 2;
	setSafeInterrupt();
	setProcessType(0x22c);
	setDaemon();
	f_10 &= ~1;
	nameProcess(pid, "Music");
	connectToRealTime(1);
	TheMusicProcess = this;
}

void MusicProcess::freeMemory(void)
{
	TheMusicProcess = 0;
	Process::freeMemory();
}

void MusicProcess::process(void)
{
	if (Music::initialized)
		Music::Music_Timer2();
}

void MusicProcess::interruptHandler(unsigned long, unsigned long)
{
	if (Music::initialized)
		Music::Music_Timer();
}

void Music::music_hook(void)
{
	drvr_desc *desc;
	int rate;

	desc = (drvr_desc *)XMI_describe_driver();
	rate = desc->service_rate;
	if (rate < 0 || rate > 1000)
		rate = 120;
	if (speedhack)
		rate = 60;
	new MusicProcess(rate);
	hooked = 1;
}

void Music::slow_stop(void)
{
	if (!initialized)
		return;
	if (current_tune < 0)
		return;
	if (current_ms.active == 1 && current_ms.handle >= 0 && music_play_state && music_stop_state
		&& XMI_sequence_status(current_ms.handle) == 2)
	{
		stop();
		current_tune = -1;
		return;
	}
	if (current_tune < 0)
		return;
	if (slow_stop_state == 0 && speedhack == 0)
		slow_stop_state = music_volume * 2;
	if (speedhack * !slow_stop_state)
		slow_stop_state = music_volume;
	if (slow_stop_state-- < 5)
	{
		stop();
		slow_stop_state = 0;
		return;
	}
	if (current_ms.active == 1 && current_ms.handle >= 0)
		XMI_set_relative_volume(current_ms.handle, slow_stop_state >> 1, 0);
	if (current_other.active == 1 && current_other.handle >= 0)
		XMI_set_relative_volume(current_other.handle, slow_stop_state >> 1, 0);
}

int Music::pushQueue(int tune)
{
	if (tune != play_queue && tune != current_tune)
		return play(tune);
	return 0;
}

void Music::Music_Timer(void)
{
	if (music_play_state && (current_ms.handle != -1 || current_other.handle != -1))
	{
		if (slow_stop_state)
			slow_stop();
		XMI_SERVE_DRIVER();
	}
}

// Switches to the next sequence once the playing one reaches the end of its measure.
void Music::Music_Timer2(void)
{
	static int busy = 0;
	char *form;

	if (busy == 1)
		return;
	busy = 1;
	if (current_ms.active && !music_status && current_ms.restart > 0 && current_ms.handle >= 0
		&& XMI_sequence_status(current_ms.handle) == 2)
	{
		if (current_tune < 0)
			current_tune = 0;
		SpeedHack(&current_ms);
		volume(music_volume, 0);
		if (XMI_branch_index(current_ms.handle, 0x7e))
			music_branch[current_tune] = 0;
	}
	if (!music_status)
	{
		busy = 0;
		return;
	}
	if (desired_tune < 0)
	{
		busy = 0;
		return;
	}
	current_tune = desired_tune;
	switch (music_status)
	{
	case 1:
		if (xwait_for_eom(&current_ms) == -1)
			break;
		XMI_stop_sequence(current_ms.handle);
		new_measure = old_measure = 0;
		current_ms.measure = 1;
		SpeedHack(&current_other);
		XMI_release_sequence_handle(current_ms.handle);
		current_ms.handle = -1;
		current_other.active = 1;
		form = Activate_Music_File(ms_files[key_array[desired_tune].file].tune);
		if (form)
			current_ms.handle = XMI_register_sequence(form, 0, current_ms.st_ptr, 0);
		ref_meas = XMI_measure_count(current_other.handle);
		music_status = 2;
		volume(music_volume, 0);
		if (current_ms.handle == -1)
			music_status = 0;
		break;
	case 2:
		if (xwait_for_eom(&current_other) == -1)
			break;
		XMI_stop_sequence(current_other.handle);
		SpeedHack(&current_ms);
		if (music_branch[current_tune] >= 0)
		{
			if (!XMI_branch_index(current_ms.handle, music_branch[current_tune]))
			{
				music_branch[current_tune] = 0;
				if (!XMI_branch_index(current_ms.handle, 0))
					music_branch[current_tune] = -1;
			}
		}
		music_branch[current_tune]++;
		XMI_release_sequence_handle(current_other.handle);
		current_other.handle = -1;
		current_ms.active = 1;
		current_ms.file = key_array[desired_tune].file;
		current_ms.restart = ms_files[current_ms.file].restart_measure;
		current_other.active = 0;
		music_status = 0;
		volume(music_volume, 0);
		break;
	case 3:
		if (xwait_for_eom(&current_ms) == -1)
			break;
		if (current_ms.handle >= 0 && current_ms.active == 1)
			XMI_stop_sequence(current_ms.handle);
		new_measure = old_measure = 0;
		current_ms.measure = 1;
		SpeedHack(&a_seq);
		if (music_branch[current_tune] >= 0)
		{
			if (!XMI_branch_index(a_seq.handle, music_branch[current_tune]))
			{
				music_branch[current_tune] = 0;
				if (!XMI_branch_index(a_seq.handle, 0))
					music_branch[current_tune] = -1;
			}
		}
		music_branch[current_tune]++;
		if (current_ms.handle >= 0)
			XMI_release_sequence_handle(current_ms.handle);
		current_ms.handle = a_seq.handle;
		farfree(current_ms.st_ptr);
		current_ms.st_ptr = a_seq.st_ptr;
		current_ms.active = 1;
		current_ms.file = key_array[desired_tune].file;
		current_ms.restart = ms_files[current_ms.file].restart_measure;
		a_seq.handle = -1;
		a_seq.st_ptr = 0;
		music_status = 0;
		volume(music_volume, 0);
		break;
	case 4:
		if (xwait_for_eom(&current_ms) == -1)
			break;
		XMI_stop_sequence(current_ms.handle);
		SpeedHack(&current_other);
		current_other.active = 1;
		ref_meas = XMI_measure_count(current_other.handle);
		music_status = 5;
		volume(music_volume, 0);
		break;
	case 5:
		if (xwait_for_eom(&current_other) == -1)
			break;
		XMI_stop_sequence(current_other.handle);
		XMI_resume_sequence(current_ms.handle);
		XMI_release_sequence_handle(current_other.handle);
		current_other.handle = -1;
		current_other.active = 0;
		music_status = 0;
		break;
	case 6:
		if (xwait_for_eom(&current_ms) == -1)
			break;
		SpeedHack(&current_other);
		current_other.active = 1;
		ref_meas = XMI_measure_count(current_other.handle);
		music_status = 7;
		volume(music_volume, 0);
		break;
	case 7:
		if (xwait_for_eom(&current_other) == -1)
			break;
		XMI_stop_sequence(current_other.handle);
		XMI_stop_sequence(current_ms.handle);
		new_measure = old_measure = 0;
		current_ms.measure = 1;
		SpeedHack(&a_seq);
		if (music_branch[current_tune] >= 0)
		{
			if (!XMI_branch_index(a_seq.handle, music_branch[current_tune]))
			{
				music_branch[current_tune] = 0;
				if (!XMI_branch_index(a_seq.handle, 0))
					music_branch[current_tune] = -1;
			}
		}
		music_branch[current_tune]++;
		XMI_release_sequence_handle(current_other.handle);
		current_other.handle = -1;
		current_other.active = 0;
		XMI_release_sequence_handle(current_ms.handle);
		current_ms.handle = -1;
		farfree(current_ms.st_ptr);
		current_ms.handle = a_seq.handle;
		current_ms.st_ptr = a_seq.st_ptr;
		current_ms.active = 1;
		current_ms.file = key_array[desired_tune].file;
		current_ms.restart = ms_files[current_ms.file].restart_measure;
		a_seq.handle = -1;
		a_seq.st_ptr = 0;
		music_status = 0;
		volume(music_volume, 0);
		break;
	case 8:
		if (xwait_for_eom(&current_ms) == -1)
			break;
		SpeedHack(&current_other);
		volume(music_volume, 0);
		music_status = 9;
		break;
	case 9:
		if (xwait_for_eom(&current_other) == -1)
			break;
		XMI_stop_sequence(current_other.handle);
		XMI_release_sequence_handle(current_other.handle);
		current_other.handle = -1;
		current_other.active = 0;
		volume(music_volume, 0);
		music_status = 0;
		break;
	}
	if (play_queue >= 0 && !music_status)
	{
		if (play(play_queue) != -123)
			play_queue = -1;
	}
	busy = 0;
}

int Music::update_music(void)
{
	static int result = 0;
	static int measureEnded;
	char *form;

	if (!initialized)
		return 0;
	if (current_ms.active == 1 && current_ms.handle >= 0 && XMI_sequence_status(current_ms.handle) == 1)
	{
		if ((measureEnded = end_measure(current_ms)) == 0 && current_tune == desired_tune)
			return -1;
	}
	if (current_ms.active == 1 && measureEnded == 1 && current_tune == desired_tune)
	{
		if (current_ms.measure % ms_files[current_ms.file].loop_measures == 0)
			current_ms.measure = current_ms.restart;
		else
			current_ms.measure++;
	}
	else
	{
		if (current_ms.active == 1 && current_ms.handle >= 0 && XMI_sequence_status(current_ms.handle) == 2)
			stop();
		if (desired_tune < 0)
			return -1;
		if (music_status)
		{
			play_queue = desired_tune;
			return -123;
		}
		if (key_array[desired_tune].used == 0)
			return -2;
		if (current_ms.handle < 0 || !current_ms.active || XMI_sequence_status(current_ms.handle) != 1)
		{
			if (key_array[desired_tune].is_tune)
			{
				if (current_ms.active)
				{
					XMI_release_sequence_handle(current_ms.handle);
					current_ms.handle = -1;
				}
				current_ms.file = key_array[desired_tune].file;
				current_ms.restart = ms_files[current_ms.file].restart_measure;
				form = Activate_Music_File(ms_files[current_ms.file].tune);
				if (form == 0)
					return -14;
				current_ms.handle = XMI_register_sequence(form, 0, current_ms.st_ptr, 0);
				if (current_ms.handle == -1)
					result = -14;
				else
				{
					SpeedHack(&current_ms);
					current_ms.active = 1;
					volume(music_volume, 0);
				}
			}
		}
		else
			result = do_normal(desired_tune);
	}
	if (TheMusicProcess)
		((Process *)TheMusicProcess)->process();
	return result;
}

void Music::deinit(void)
{
	if (!initialized)
		return;
	if (!hooked)
		return;
	stop();
	AIL_shutdown(0);
	XMI_shutdown_driver("ACM ULTIMA 8 MCA");
	Music_Flex.close();
	hooked = 0;
	initialized = 0;
	last_read[0] = last_read[1] = 0;
	if (Music_Index_Ptr)
	{
		delete Music_Index_Ptr;
		Music_Index_Ptr = 0;
	}
	if (TheMusicProcess)
		((Process *)TheMusicProcess)->pop(0);
	if (trans_table)
	{
		delete trans_table;
		trans_table = 0;
	}
	if (table_table)
	{
		delete table_table;
		table_table = 0;
	}
	if (ms_files)
	{
		delete ms_files;
		ms_files = 0;
	}
	if (key_array)
	{
		delete key_array;
		key_array = 0;
	}
	if (current_other.st_ptr)
	{
		delete current_other.st_ptr;
		current_other.st_ptr = 0;
	}
	if (current_ms.st_ptr)
	{
		delete current_ms.st_ptr;
		current_ms.st_ptr = 0;
	}
	if (a_seq.st_ptr)
	{
		delete a_seq.st_ptr;
		a_seq.st_ptr = 0;
	}
	if (trans_file_ptr)
	{
		delete trans_file_ptr;
		trans_file_ptr = 0;
	}
	if (Music_Buffer[0])
	{
		delete Music_Buffer[0];
		Music_Buffer[0] = 0;
	}
	if (Music_Buffer[1])
	{
		delete Music_Buffer[1];
		Music_Buffer[1] = 0;
	}
	if (deleteMe)
	{
		delete deleteMe;
		deleteMe = 0;
	}
	music_play_state = 0;
	music_stop_state = 0;
}

void Music::reset(void)
{
	for (int i = 0; i < 255; i++)
		music_branch[i] = -1;
}

int Music::status(void)
{
	if (!initialized)
		return -1;
	if (current_tune < 0)
		return -1;
	if (current_ms.active == 1 && music_play_state && music_stop_state
		&& (!slow_stop_state || slow_stop_state > 5) && current_ms.active && current_ms.handle >= 0
		&& XMI_sequence_status(current_ms.handle) == 2)
		current_tune = -1;
	if (music_status)
		return desired_tune;
	if (!current_ms.active && !current_other.active)
	{
		current_tune = -1;
		desired_tune = -1;
	}
	return current_tune;
}

void Music::volume(int level, int time)
{
	if (level >= 0 && level <= 100)
	{
		if (current_ms.active == 1 && current_ms.handle >= 0)
			XMI_set_relative_volume(current_ms.handle, level, time);
		if (current_other.active == 1 && current_other.handle >= 0)
			XMI_set_relative_volume(current_other.handle, level, time);
		music_volume = level;
	}
}

void musicVolume(int level, int time)
{
	Music::volume(level, time);
}

void musicPlay(int tune)
{
	Music::play(tune);
}

void musicDeinit(void)
{
	Music::deinit();
}

void musicStatus(void)
{
	Music::status();
}

void musicPause(void)
{
	Music::pause();
}

void musicResume(void)
{
	Music::resume();
}

void musicStop(void)
{
	Music::stop();
}

void musicSlowStop(void)
{
	Music::slow_stop();
}

void musicReset(void)
{
	Music::reset();
}

void stackPlay(int tune)
{
	Music::pushQueue(tune);
}

void musicInit(char *name, int port, int irq, int dma, int drq)
{
	Music::init(name, port, irq, dma, drq);
}

// Two buffers hold the last two files read, so a transition can reuse the one still playing.
char *Music::Activate_Music_File(int tune)
{
	static int which = 0;
	char *buffer = 0;
	char *result = 0;
	unsigned size;

	result = Music_Buffer[0];
	if (result == 0)
		result = Music_Buffer[1];
	if (!mpu_401_device)
		tune += 128;
	if (tune < 0)
		return result;
	if (last_read[0] == tune)
	{
		which = 1;
		return Music_Buffer[0];
	}
	if (last_read[1] == tune)
	{
		which = 0;
		return Music_Buffer[1];
	}
	Music_Flex.getIndex(tune, Music_Index);
	size = Music_Index.size;
	if (!mpu_401_device && size < 1 && initialized)
	{
		tune -= 128;
		Music_Flex.getIndex(tune, Music_Index);
		size = Music_Index.size;
	}
	if (size < 1)
	{
		if (Music_Buffer[0])
		{
			which = 1;
			return Music_Buffer[0];
		}
		if (Music_Buffer[1])
		{
			which = 0;
			return Music_Buffer[1];
		}
		return result;
	}
	switch (which)
	{
	case 0:
		which++;
		buffer = new char[size + 1];
		last_read[0] = tune;
		if (buffer == 0)
			return Music_Buffer[0];
		if (Music_Buffer[0])
		{
			delete Music_Buffer[0];
			Music_Buffer[0] = 0;
		}
		Music_Buffer[0] = buffer;
		Music_Flex.readRecord(tune, Music_Buffer[0]);
		return Music_Buffer[0];
	case 1:
		which--;
		buffer = new char[size + 1];
		last_read[1] = tune;
		if (buffer == 0)
			return Music_Buffer[1];
		if (Music_Buffer[1])
		{
			delete Music_Buffer[1];
			Music_Buffer[1] = 0;
		}
		Music_Buffer[1] = buffer;
		Music_Flex.readRecord(tune, Music_Buffer[1]);
		return Music_Buffer[1];
	}
	if (result == 0)
		result = Music_Buffer[1];
	return result;
}

void Music::SpeedHack(current_type *seq)
{
	if (seq->handle < 0)
		return;
	XMI_start_sequence(seq->handle);
	if (speedhack)
		XMI_set_relative_tempo(seq->handle, 200, 0);
}

static char nim_file[] = "GENERIC.NIM";
char GTL_PREFIX[6] = "XMIDI";
