/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ virtual machine ]
*/

#ifndef _MZ700_H_
#define _MZ700_H_

#if defined(_MZ700)
#define DEVICE_NAME		"SHARP MZ-700"
#define CONFIG_NAME		"mz700"
#elif defined(_MZ1500)
#define DEVICE_NAME		"SHARP MZ-1500"
#define CONFIG_NAME		"mz1500"
#endif

// device informations for virtual machine
#if defined(_MZ700) && defined(_PAL)
#define LPHI_CLOCKS			17734475
#define CPU_CLOCKS			(LPHI_CLOCKS/5)
#define LINES_PER_FRAME		312
#define FRAMES_PER_SEC		(CPU_CLOCKS / 228.0 / LINES_PER_FRAME)
#else
#define CPU_CLOCKS			3579545
#define LINES_PER_FRAME		262
#define FRAMES_PER_SEC		(CPU_CLOCKS / 228.0 / LINES_PER_FRAME)
#endif
#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define WINDOW_HEIGHT_ASPECT	480
#define IO_ADDR_MAX		0x100
#define Z80_MEMORY_WAIT
#define Z80_IO_WAIT
#if defined(_MZ1500)
#define MAX_DRIVE		4
#define HAS_MB8876
#endif
#if defined(_MZ1500)
#define MZ1P17_SW1_4_ON
#endif

// device informations for win32
#if defined(_MZ700)
#define USE_DIPSWITCH
#define USE_COLOR_BLENDER
#endif
#define USE_TAPE		1
#define USE_ROMDISK
#if defined(_MZ1500)
#define USE_FLOPPY_DISK		2
#define USE_QUICK_DISK		1
#endif
#define USE_AUTO_KEY		5
#define USE_AUTO_KEY_RELEASE	6
#define USE_AUTO_KEY_CAPS
#define USE_AUTO_KEY_NUMPAD
#define USE_VM_AUTO_KEY_TABLE
#define USE_SCREEN_FILTER
#define USE_SCANLINE
#if defined(_MZ700)
#define USE_SOUND_VOLUME	3
#elif defined(_MZ1500)
#define USE_SOUND_VOLUME	6
#endif
#if defined(_MZ1500)
#define USE_PRINTER
#define USE_PRINTER_TYPE	4
#endif
#define USE_DEBUGGER
#define USE_STATE

static const int vm_auto_key_table_base[][2] = {
	// thanks Mr.Koucha Youkan
	{0xa1,	0x300 | 0xbe},	// '�'    *** MODIFIED ***
	{0xa2,	0x300 | 0x38},	// '�'    *** MODIFIED ***
	{0xa3,	0x300 | 0x39},	// '�'    *** MODIFIED ***
	{0xa4,	0x300 | 0xbc},	// '�'    *** MODIFIED ***
	{0xa5,	0x300 | 0x30},	// '�'    *** MODIFIED ***
	{0xa6,	0x200 | 0xdb},	// '�'    *** MODIFIED ***
	{0xa7,	0x300 | 0x31},	// '�'
	{0xa8,	0x300 | 0x32},	// '�'
	{0xa9,	0x300 | 0x33},	// '�'
	{0xaa,	0x300 | 0x34},	// '�'
	{0xab,	0x300 | 0x35},	// '�'
	{0xac,	0x300 | 0xbd},	// '�'    *** MODIFIED ***
	{0xad,	0x300 | 0xde},	// '�'    *** MODIFIED ***
	{0xae,	0x300 | 0xdc},	// '�'    *** MODIFIED ***
	{0xaf,	0x300 | 0x43},	// '�'
	{0xb0,	0x200 | 0xe2},	// '�'    *** MODIFIED ***
	{0xb1,	0x200 | 0x31},	// '�'
	{0xb2,	0x200 | 0x32},	// '�'
	{0xb3,	0x200 | 0x33},	// '�'
	{0xb4,	0x200 | 0x34},	// '�'
	{0xb5,	0x200 | 0x35},	// '�'
	{0xb6,	0x200 | 0x51},	// '�'
	{0xb7,	0x200 | 0x57},	// '�'
	{0xb8,	0x200 | 0x45},	// '�'
	{0xb9,	0x200 | 0x52},	// '�'
	{0xba,	0x200 | 0x54},	// '�'
	{0xbb,	0x200 | 0x41},	// '�'
	{0xbc,	0x200 | 0x53},	// '�'
	{0xbd,	0x200 | 0x44},	// '�'
	{0xbe,	0x200 | 0x46},	// '�'
	{0xbf,	0x200 | 0x47},	// '�'
	{0xc0,	0x200 | 0x5a},	// '�'
	{0xc1,	0x200 | 0x58},	// '�'
	{0xc2,	0x200 | 0x43},	// '�'
	{0xc3,	0x200 | 0x56},	// '�'
	{0xc4,	0x200 | 0x42},	// '�'
	{0xc5,	0x200 | 0x36},	// '�'
	{0xc6,	0x200 | 0x37},	// '�'
	{0xc7,	0x200 | 0x38},	// '�'
	{0xc8,	0x200 | 0x39},	// '�'
	{0xc9,	0x200 | 0x30},	// '�'
	{0xca,	0x200 | 0x59},	// '�'
	{0xcb,	0x200 | 0x55},	// '�'
	{0xcc,	0x200 | 0x49},	// '�'
	{0xcd,	0x200 | 0x4f},	// '�'
	{0xce,	0x200 | 0x50},	// '�'
	{0xcf,	0x200 | 0x48},	// '�'
	{0xd0,	0x200 | 0x4a},	// '�'
	{0xd1,	0x200 | 0x4b},	// '�'
	{0xd2,	0x200 | 0x4c},	// '�'
	{0xd3,	0x200 | 0xbb},	// '�'
	{0xd4,	0x200 | 0xbd},	// '�'    *** MODIFIED ***
	{0xd5,	0x200 | 0xde},	// '�'    *** MODIFIED ***
	{0xd6,	0x200 | 0xdc},	// '�'    *** MODIFIED ***
	{0xd7,	0x200 | 0x4e},	// '�'    *** MODIFIED ***
	{0xd8,	0x200 | 0x4d},	// '�'    *** MODIFIED ***
	{0xd9,	0x200 | 0xbc},	// '�'    *** MODIFIED ***
	{0xda,	0x200 | 0xbe},	// '�'    *** MODIFIED ***
	{0xdb,	0x200 | 0xbf},	// '�'    *** MODIFIED ***
	{0xdc,	0x200 | 0xc0},	// '�'    *** MODIFIED ***
	{0xdd,	0x200 | 0x78},	// '�'    *** MODIFIED ***
	{0xde,	0x200 | 0xba},	// '�'    *** MODIFIED ***
	{0xdf,	0x200 | 0xdd},	// '�'    *** MODIFIED ***
	{-1,	-1},
};

#include "../../common.h"
#include "../../fileio.h"
#include "../vm_template.h"

#ifdef USE_SOUND_VOLUME
static const _TCHAR *sound_device_caption[] = {
#if defined(_MZ1500)
	_T("PSG #1"), _T("PSG #2"),
#endif
	_T("Beep"), _T("CMT (Signal)"),
#if defined(_MZ1500)
	_T("Noise (FDD)"),
#endif
	_T("Noise (CMT)"),
};
#endif

#ifdef USE_JOY_BUTTON_CAPTIONS
static const _TCHAR *joy_button_captions[] = {
	_T("Up"),
	_T("Down"),
	_T("Left"),
	_T("Right"),
	_T("Button #1"),
	_T("Button #2"),
	_T("Run"),
	_T("Select"),
};
#endif

class EMU;
class DEVICE;
class EVENT;

class AND;
class DATAREC;
class I8253;
class I8255;
class IO;
class PCM1BIT;
class Z80;

//class CMOS;
class EMM;
class KANJI;
class KEYBOARD;
class MEMORY;
class RAMFILE;
#if defined(USE_ROMDISK)
class SST39SF040;
#endif
#if defined(_MZ1500)
class MB8877;
class NOT;
class SN76489AN;
class Z80PIO;
class Z80SIO;
class FLOPPY;
class PSG;
class QUICKDISK;
#endif
class JOYSTICK;

class VM : public VM_TEMPLATE
{
protected:
//	EMU* emu;
	
	// devices
	EVENT* event;
	
	AND* and_int;
	DATAREC* drec;
	I8253* pit;
	I8255* pio;
	IO* io;
	PCM1BIT* pcm;
	Z80* cpu;
	
//	CMOS* cmos;
	EMM* emm;
	KANJI* kanji;
	KEYBOARD* keyboard;
	MEMORY* memory;
	RAMFILE* ramfile;

#if defined(USE_ROMDISK)
	SST39SF040* sst39sf040;
#endif

#if defined(_MZ1500)
	AND* and_snd;
	MB8877* fdc;
	DEVICE* printer;
	NOT* not_reset;
	NOT* not_strobe;
	SN76489AN* psg_l;
	SN76489AN* psg_r;
	Z80PIO* pio_int;
	Z80SIO* sio_rs;	// RS-232C
	Z80SIO* sio_qd;	// QD
	
	FLOPPY* floppy;
	PSG* psg;
	QUICKDISK* qd;
#endif
	JOYSTICK* joystick;
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	
	VM(EMU* parent_emu);
	~VM();
	
	// ----------------------------------------
	// for emulation class
	// ----------------------------------------
	
	// drive virtual machine
	void reset();
	void run();
	double get_frame_rate()
	{
		return FRAMES_PER_SEC;
	}
	
#ifdef USE_DEBUGGER
	// debugger
	DEVICE *get_cpu(int index);
#endif
	
	// draw screen
	void draw_screen();
	
	// sound generation
	void initialize_sound(int rate, int samples);
	uint16_t* create_sound(int* extra_frames);
	int get_sound_buffer_ptr();
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	
	// user interface
	void play_tape(int drv, const _TCHAR* file_path);
	void rec_tape(int drv, const _TCHAR* file_path);
	void close_tape(int drv);
	bool is_tape_inserted(int drv);
	bool is_tape_playing(int drv);
	bool is_tape_recording(int drv);
	int get_tape_position(int drv);
	const _TCHAR* get_tape_message(int drv);
	void push_play(int drv);
	void push_stop(int drv);
	void push_fast_forward(int drv);
	void push_fast_rewind(int drv);
	void push_apss_forward(int drv) {}
	void push_apss_rewind(int drv) {}
#if defined(_MZ1500)
	void open_quick_disk(int drv, const _TCHAR* file_path);
	void close_quick_disk(int drv);
	bool is_quick_disk_inserted(int drv);
	uint32_t is_quick_disk_accessed();
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
	uint32_t is_floppy_disk_accessed();
#endif
	bool is_frame_skippable();
	
	void update_config();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// ----------------------------------------
	// for each device
	// ----------------------------------------
	
	// devices
	DEVICE* get_device(int id);
};

#endif
