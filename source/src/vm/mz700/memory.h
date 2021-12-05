/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu, *d_pit, *d_pio;
	DEVICE* d_joystick;
#if defined(USE_ROMDISK)
	DEVICE* d_romdisk[1];
#endif
	
	// memory
	uint8_t* rbank[32];
	uint8_t* wbank[32];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	
	uint8_t ipl[0x1000];	// IPL 4KB
#if defined(_MZ1500)
	uint8_t ext[0x1800];	// MZ-1500 EXT 6KB
#endif
	uint8_t font[0x2000];	// CGROM 4KB / XCGROM 8KB
#if defined(_MZ700)
	uint8_t pcg[0x1000];	// PCG-700 2KB + Lower CGROM 2KB
#elif defined(_MZ1500)
	uint8_t pcg[0x6000];	// MZ-1500 PCG 8KB * 3
#endif
	uint8_t ram[0x10000];	// Main RAM 64KB
	uint8_t vram[0x1000];	// MZ-700/1500 VRAM 4KB
	uint8_t mem_bank;
#if defined(_MZ700)
	uint8_t pcg_data;
	uint8_t pcg_addr;
	uint8_t pcg_ctrl;
#elif defined(_MZ1500)
	uint8_t pcg_bank;
#endif
#if defined(USE_ROMDISK)
	int8_t ipl_storage;             // 0: Normal IPL 4KB, 1: FLASH IPL+DISK 512KB
	int8_t ipl_page;
#endif
	void update_map_low();
	void update_map_high();
	
	// crtc
#if defined(_MZ1500)
	uint8_t priority, palette[8];
#endif
	bool blink, tempo;
	bool blank;
	bool hblank, hsync;
	bool vblank, vsync;
	bool blank_vram;
#if defined(_MZ1500)
	bool hblank_pcg;
#endif
	void set_vblank(bool val);
	void set_hblank(bool val);
	void set_blank(bool val);

	// renderer
	uint8_t screen[200][320];
	uint8_t screen_copy[200][320];
	scrntype_t palette_pc[8];
	
	FILEIO* log = nullptr;

	void draw_line(int v);
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
#if defined(USE_ROMDISK)
	void update_config();
#endif
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data8w(uint32_t addr, int* wait);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pit(DEVICE* device)
	{
		d_pit = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
#if defined(USE_ROMDISK)
	void set_context_romdisk(int i, DEVICE* device)
	{
		if (unsigned(i) < 2) {
			d_romdisk[i] = device;
		}
	}
#endif
	void draw_screen();
};

#endif

