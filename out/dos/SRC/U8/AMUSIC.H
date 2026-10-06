#ifndef AMUSIC_H
#define AMUSIC_H

#include "..\GLIB\PROCESS.H"
#include "CINTLIST.H"
#include "FASTFLEX.H"
#include "AIL.H"

// What XMI_describe_driver points to.
struct drvr_desc
{
	unsigned min_API_version;
	unsigned drvr_type;
	char data_suffix[4];
	void *dev_name_table;
	int default_IO;
	int default_IRQ;
	int default_DMA;
	int default_DRQ;
	int service_rate;
	unsigned display_size;
};

// One playing XMIDI sequence.
class current_type
{
public:
	int measure;
	int restart;
	int file;
	int handle;
	char *st_ptr;
	int active;
};

// One music file listed in the .NIM file.
class file_info_type
{
public:
	char name[80];
	char *data;
	char tune;
	char pad_55;
	int loop_measures;
	int restart_measure;
};

// What each tune key is bound to.
class key_type
{
public:
	int file;
	int is_tune;
	int used;
	int pad_6[3];
};

class Music
{
public:
	static char skip_blanks(BaseFile *);
	static void skip_to_eol(BaseFile *);
	static int read_to_white(BaseFile *, char *);
	static int load_timbres(file_info_type *, int, unsigned char *, unsigned char *);
	static int first_section(file_info_type *);
	static char *second_section(void);
	static int third_section(void);
	static int do_data_line(BaseFile *, int, int (*)[100]);
	static int fourth_section(file_info_type *, int);
	static int fifth_section(file_info_type *, int);
	static char *do_files(file_info_type *, short *, char *);
	static char *load_global_timbre(unsigned, unsigned);
	static void Music_halt(char *, int, int, char *);
	static int init(char *, int, int, int, int);
	static int play(int);
	static int resume(void);
	static int pause(void);
	static void stop(void);
	static int xwait_for_eom(current_type *);
	static int end_measure(current_type);
	static int do_normal(int);
	static int init_AIL(long *);
	static void music_hook(void);
	static void slow_stop(void);
	static int pushQueue(int);
	static void Music_Timer(void);
	static void Music_Timer2(void);
	static int update_music(void);
	static void deinit(void);
	static void reset(void);
	static int status(void);
	static void volume(int, int);
	static char *Activate_Music_File(int);
	static void SpeedHack(current_type *);
	static int speedhack;
	static int music_volume;
	static char *Music_Buffer[2];
	static Index *Music_Index_Ptr;
	static int last_read[2];
	static unsigned char hdriver[2];
	static long state_table_size;
	static int old_measure;
	static int new_measure;
	static int (*trans_table)[100];
	static int (*table_table)[256];
	static int desired_tune;
	static int current_tune;
	static int initialized;
	static int hooked;
	static short num_ms;
	static int music_status;
	static int ref_meas;
	static int music_play_state;
	static int music_stop_state;
	static int slow_stop_state;
	static int mpu_401_device;
	static int auto_detect;
	static int music_dma;
	static int music_irq;
	static int music_drq;
	static int music_port;
	static char music_branch[256];
	static FastFlex Music_Flex;
	static Index Music_Index;
	static char *trans_file_ptr;
	static int play_queue;
	static file_info_type *ms_files;
	static key_type *key_array;
	static current_type current_ms;
	static current_type current_other;
	static current_type *current_temp;
	static current_type a_seq;
};

class MusicProcess : public ProcessRealTime
{
public:
	MusicProcess(short);
	virtual void process(void);
	virtual void freeMemory(void);
	virtual void interruptHandler(unsigned long, unsigned long);
};

void musicVolume(int, int);
void musicPlay(int);
void musicDeinit(void);
void musicStatus(void);
void musicPause(void);
void musicResume(void);
void musicStop(void);
void musicSlowStop(void);
void musicReset(void);
void stackPlay(int);
void musicInit(char *, int, int, int, int);

extern void *TheMusicProcess;

#endif
