/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ memory ]
*/

#include "memory.h"
#include "../i8253.h"
#include "../i8255.h"

#if defined(_PAL)
// need to be revised as the BLNK and /HBLK start at the same clock pulse. 
#define BLANK_S				147
#define BLANK_E				0
#define HBLANK_S			128
#define HBLANK_E			0
#define HSYNC_S				160
#define HSYNC_E				176
#define VSYNC_S				245
#define VSYNC_E				247
#else
// http://www.maroon.dti.ne.jp/youkan/mz700/M60719/htiming.html
#define BLANK_S				160	// 640 / 4 = 160
#define BLANK_E				0	//   0 / 4 = 0
#define HBLANK_S			160	// 640 / 4 = 160
#define HBLANK_E			220	// 881 / 4 = 220.25
#define HSYNC_S				180	// 720 / 4 = 180
#define HSYNC_E				196	// 785 / 4 = 196.25
#define VSYNC_S				221
#define VSYNC_E				223
#endif

#define EVENT_TEMPO			0
#define EVENT_BLINK			1
#define EVENT_BLANK_S		2
#define EVENT_BLANK_E		3
#define EVENT_HBLANK_S		4
#define EVENT_HBLANK_E		5
#define EVENT_HSYNC_S		6
#define EVENT_HSYNC_E		7
#if defined(_MZ1500)
#define EVENT_HBLANK_PCG_S	8
#endif

#define MEM_BANK_MON_L		0x01
#define MEM_BANK_MON_H		0x02
#if defined(_MZ1500)
#define MEM_BANK_PCG		0x20
#endif

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

#define DEFAULT_IPLROM_FILE_NAME	"IPL.ROM"
#define DEFAULT_EXTROM_FILE_NAME	"EXT.ROM"
#define DEFAULT_XCGROM_FILE_NAME	"XCG.ROM"
#if defined(_PAL)
#define IPLROM_FILE_NAME	"IPL-EU.ROM"
#define EXTROM_FILE_NAME	"EXT-EU.ROM"
#define XCGROM_FILE_NAME	"XCG-EU.ROM"
#else
#define IPLROM_FILE_NAME	"IPL-JP.ROM"
#define EXTROM_FILE_NAME	"EXT-JP.ROM"
#define XCGROM_FILE_NAME	"XCG-JP.ROM"
#endif

void MEMORY::initialize()
{
	// init memory
	memset(ipl, 0xff, sizeof(ipl));
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
	memset(vram + 0x800, 0x71, 0x400);
#if defined(_MZ1500)
	memset(ext, 0xff, sizeof(ext));
	memset(pcg, 0, sizeof(pcg));
#endif
	memset(font, 0, sizeof(font));
	memset(rdmy, 0xff, sizeof(rdmy));

	// load rom images
	FILEIO* fio = new FILEIO();
	if (fio->Fopen(create_local_path(_T(IPLROM_FILE_NAME)), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	} else if (fio->Fopen(create_local_path(_T(DEFAULT_IPLROM_FILE_NAME)), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
#if defined(_MZ1500)
	if(fio->Fopen(create_local_path(_T(EXT_FILE_NAME)), FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	} else if (fio->Fopen(create_local_path(_T(DEFAULT_EXT_FILE_NAME)), FILEIO_READ_BINARY)) {
		fio->Fread(ext, sizeof(ext), 1);
		fio->Fclose();
	}
#endif
	if(fio->Fopen(create_local_path(_T(XCGROM_FILE_NAME)), FILEIO_READ_BINARY)) {
		if (fio->Fread(font, sizeof(font), 1) < sizeof(font)) {
			memcpy(font + 0x1000, font, sizeof(font) / 2);
		};
		fio->Fclose();
	} else if (fio->Fopen(create_local_path(_T(DEFAULT_XCGROM_FILE_NAME)), FILEIO_READ_BINARY)) {
		if (fio->Fread(font, sizeof(font), 1) < sizeof(font)) {
			memcpy(font + 0x1000, font, sizeof(font) / 2);
		};
		fio->Fclose();
	}

#if defined(USE_ROMDISK)
	ipl_page = 0;
	update_config();
#endif

	delete fio;
	
#if defined(_MZ700)
	// init PCG-700
	memset(pcg, 0, sizeof(pcg));
	memcpy(pcg + 0x000, font + 0x000, 0x400);
	memcpy(pcg + 0x800, font + 0x800, 0x400);
#endif
	
	// init memory map
	SET_BANK(0x0000, 0xffff, ram, ram);
	
	// create pc palette
	for(int i = 0; i < 8; i++) {
		palette_pc[i] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);
	}

	// register event
	register_vline_event(this);
	register_event_by_clock(this, EVENT_TEMPO, CPU_CLOCKS / 64, true, NULL);	// 32hz * 2
	register_event_by_clock(this, EVENT_BLINK, CPU_CLOCKS / 3, true, NULL);	// 1.5hz * 2
}

void MEMORY::release()
{
}

void MEMORY::reset()
{
	// reset memory map
	mem_bank = MEM_BANK_MON_L | MEM_BANK_MON_H;
#if defined(_MZ1500)
	pcg_bank = 0;
#endif
	update_map_low();
	update_map_high();
	
	// reset crtc
	blink = tempo = false;
	vblank = vsync = true;
	hblank = hsync = true;
	blank_vram = true;
#if defined(_MZ1500)
	hblank_pcg = true;
#endif
	
#if defined(_MZ700)
	// reset PCG-700
	pcg_data = pcg_addr = 0;
	pcg_ctrl = 0xff;
#endif
	
	// reset palette
#if defined(_MZ1500)
	for(int i = 0; i < 8; i++) {
		palette[i] = i;
	}
#endif

#if defined(USE_ROMDISK)
	// reset ROM/DISK page selector
	ipl_storage = uint8_t(config.ipl_storage);
#endif

	// motor is always rotating...
	d_pio->write_signal(SIG_I8255_PORT_C, 0xff, 0x10);
}

#if defined(USE_ROMDISK)
void MEMORY::update_config()
{
	ipl_storage = uint8_t(config.ipl_storage);
}
#endif

void MEMORY::event_vline(int v, int clock)
{
	// vblank / vsync
	set_vblank(v >= 200);
	vsync = (v >= VSYNC_S && v <= VSYNC_E);
	
	// hblank / hsync
	set_hblank(false);
	set_blank(false);
	register_event_by_clock(this, EVENT_HBLANK_S, HBLANK_S	, false, NULL);
	register_event_by_clock(this, EVENT_BLANK_S	, BLANK_S	, false, NULL);
	register_event_by_clock(this, EVENT_BLANK_E	, BLANK_E	, false, NULL);
	register_event_by_clock(this, EVENT_HSYNC_S	, HSYNC_S	, false, NULL);
	register_event_by_clock(this, EVENT_HSYNC_E	, HSYNC_E	, false, NULL);
#if defined(_MZ1500)
	// memory wait for pcg
	register_event_by_clock(this, EVENT_HBLANK_PCG_S, 170, false, NULL);
	hblank_pcg = false;
#endif

	// memory wait for vram
	blank_vram = false;
	
	// draw one line
	if(v < 200) {
		draw_line(v);
	}
}

void MEMORY::event_callback(int event_id, int err)
{
	if(event_id == EVENT_TEMPO) {
		// 32KHz
		tempo = !tempo;
	} else if(event_id == EVENT_BLINK) {
		// 556 OUT (1.5KHz) -> 8255:PC6
		d_pio->write_signal(SIG_I8255_PORT_C, (blink = !blink) ? 0xff : 0, 0x40);
	} else if(event_id == EVENT_HBLANK_S) {
		set_hblank(true);
	} else if(event_id == EVENT_BLANK_S) {
		set_blank(true);
		blank_vram = true;
	} else if(event_id == EVENT_BLANK_E) {
		set_blank(false);
		blank_vram = false;
	} else if(event_id == EVENT_HSYNC_S) {
		hsync = true;
	} else if(event_id == EVENT_HSYNC_E) {
		hsync = false;
#if defined(_MZ1500)
	} else if(event_id == EVENT_HBLANK_PCG_S) {
		if(hblank_pcg) {
			// wait because pcg is accessed
			d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
		}
		hblank_pcg = true;
#endif
	}
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	// MZ-700/1500
#if defined(_MZ1500)
	if(mem_bank & MEM_BANK_PCG) {
		if(0xd000 <= addr && addr <= 0xefff) {
			// pcg wait
			if(!hblank_pcg) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				hblank_pcg = true;
			}
		}
	} else {
#endif
		if(mem_bank & MEM_BANK_MON_H) {
			if(0xe000 <= addr && addr <= 0xe00f) {
				// memory mapped i/o
				switch(addr & 0x0f) {
				case 0: case 1: case 2: case 3:
					d_pio->write_io8(addr & 3, data);
					break;
				case 4: case 5: case 6: case 7:
					d_pit->write_io8(addr & 3, data);
					break;
				case 8:
					// 8253 gate0
					d_pit->write_signal(SIG_I8253_GATE_0, data, 1);
					break;
				}
				return;
#if defined(_MZ700)
			} else if(addr == 0xe010) {
				pcg_data = data;
				return;
			} else if(addr == 0xe011) {
				pcg_addr = data;
				return;
			} else if(addr == 0xe012) {
				if(!(pcg_ctrl & 0x10) && (data & 0x10)) {
					int offset = pcg_addr | ((data & 3) << 8);
					offset |= (data & 4) ? 0xc00 : 0x400;
					pcg[offset] = (data & 0x20) ? font[offset] : pcg_data;
				}
				pcg_ctrl = data;
				return;
#endif
			}
		}
#if defined(_MZ1500)
	}
#endif
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	// MZ-700/1500
#if defined(_MZ1500)
	if(mem_bank & MEM_BANK_PCG) {
		if(0xd000 <= addr && addr <= 0xefff) {
			// pcg wait
			if(!hblank_pcg) {
				d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
				hblank_pcg = true;
			}
		}
	} else {
#endif
		if(mem_bank & MEM_BANK_MON_H) {
			if(0xe000 <= addr && addr <= 0xe00f) {
				// memory mapped i/o
				switch(addr & 0x0f) {
				case 0: case 1: case 2: case 3:
					return d_pio->read_io8(addr & 3);
				case 4: case 5: case 6: case 7:
					return d_pit->read_io8(addr & 3);
				case 8:
					return (hblank ? 0 : 0x80) | (tempo ? 1 : 0) | 0x7e;
				}
				return 0xff;
			}
		}
#if defined(_MZ1500)
	}
#endif
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	*wait = 0;
	if(mem_bank & MEM_BANK_MON_L && (addr < 0x1000)) {
		*wait = 1;
#if defined(USE_ROMDISK)
		switch(ipl_storage) {
		case 1: // flash 512KB
			d_romdisk[ipl_storage - 1]->write_data8((ipl_page & 255) * 0x1000 + addr, data);
			return;
		default:
			break;
		}
#endif
	} else if(mem_bank & MEM_BANK_MON_H && (0xd000 <= addr && addr <= 0xdfff)) {
		int left = BLANK_S - int(get_passed_clock_since_vline());
		if (left > 0) {
			int delta = d_cpu->get_tstates() + 2; // T-states of previous M-cycle + T2
			*wait = left + delta;
		}
	}
	write_data8(addr, data);
}

uint32_t MEMORY::read_data8w(uint32_t addr, int* wait)
{
	*wait = 0;
	if((mem_bank & MEM_BANK_MON_L) && addr < 0x1000) {
		*wait = 1;
#if defined(USE_ROMDISK)
		switch(ipl_storage) {
		case 1: // flash 512KB
			return d_romdisk[ipl_storage - 1]->read_data8((ipl_page & 255) * 0x1000 + addr);
		default:
			break;
		}
#endif
	} else if(mem_bank & MEM_BANK_MON_H && (0xd000 <= addr && addr <= 0xdfff)) {
		int left = BLANK_S - int(get_passed_clock_since_vline());
		if(left > 0) {
			int delta = d_cpu->get_tstates() + 2; // T-states of previous M-cycle + T2
			*wait = left + delta;
		}
	}
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch (addr & 0xff) {
	case 0xe0:
		mem_bank &= ~MEM_BANK_MON_L;
		update_map_low();
		break;
	case 0xe1:
		mem_bank &= ~MEM_BANK_MON_H;
		update_map_high();
		break;
	case 0xe2:
		mem_bank |= MEM_BANK_MON_L;
		update_map_low();
		break;
	case 0xe3:
		mem_bank |= MEM_BANK_MON_H;
		update_map_high();
		break;
	case 0xe4:
		mem_bank |= MEM_BANK_MON_L | MEM_BANK_MON_H;
#if defined(_MZ1500)
		mem_bank &= ~MEM_BANK_PCG;
#endif
		update_map_low();
		update_map_high();
		break;
#if defined(_MZ1500)
	case 0xe5:
		mem_bank |= MEM_BANK_PCG;
		pcg_bank = data;
		update_map_high();
		break;
	case 0xe6:
		mem_bank &= ~MEM_BANK_PCG;
		update_map_high();
		break;
	case 0xf0:
		priority = data;
		break;
	case 0xf1:
		palette[(data >> 4) & 7] = data & 7;
		break;
#endif
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xff:
		ipl_page = uint8_t(addr >> 8);
		break;
	}
	return 0xff;
}

void MEMORY::set_vblank(bool val)
{
	if(vblank != val) {
		// VBLANK -> 8255:PC7
		d_pio->write_signal(SIG_I8255_PORT_C, val ? 0 : 0xff, 0x80);
		vblank = val;
	}
}

void MEMORY::set_hblank(bool val)
{
	if(hblank != val) {
		hblank = val;
	}
}

void MEMORY::set_blank(bool val)
{
	if (blank != val) {
		// BLANK -> 8253:CLK1 TODO
		d_pit->write_signal(SIG_I8253_CLOCK_1, val ? 0 : 0xff, 0x20);
		blank = val;
	}
}

void MEMORY::update_map_low()
{
	if(mem_bank & MEM_BANK_MON_L) {
		SET_BANK(0x0000, 0x0fff, wdmy, ipl);
	} else {
		SET_BANK(0x0000, 0x0fff, ram, ram);
	}
}

void MEMORY::update_map_high()
{
	// MZ-700/1500
#if defined(_MZ1500)
	if(mem_bank & MEM_BANK_PCG) {
		if(pcg_bank & 3) {
			uint8_t *bank = pcg + ((pcg_bank & 3) - 1) * 0x2000;
			SET_BANK(0xd000, 0xefff, bank, bank);
		} else {
			SET_BANK(0xd000, 0xdfff, wdmy, font);	// read only
			SET_BANK(0xe000, 0xefff, wdmy, font);
		}
		SET_BANK(0xf000, 0xffff, wdmy, rdmy);
	} else {
#endif
		if(mem_bank & MEM_BANK_MON_H) {
			SET_BANK(0xd000, 0xdfff, vram, vram);
#if defined(_MZ1500)
			SET_BANK(0xe000, 0xe7ff, wdmy, rdmy);
			SET_BANK(0xe800, 0xffff, wdmy, ext );
#else
			SET_BANK(0xe000, 0xffff, wdmy, rdmy);
#endif
		} else {
			SET_BANK(0xd000, 0xffff, ram + 0xd000, ram + 0xd000);
		}
#if defined(_MZ1500)
	}
#endif
}

void MEMORY::draw_line(int v)
{
	int ptr = 40 * (v >> 3);
#if defined(_MZ700)
	bool pcg_active = ((config.dipswitch & 1) && !(pcg_ctrl & 8));
#endif
	
	for(int x = 0; x < 320; x += 8) {
		uint8_t attr = vram[ptr | 0x800];
#if defined(_MZ1500)
		uint8_t pcg_attr = vram[ptr | 0xc00];
#endif
#if defined(_MZ700)
		uint16_t code = (vram[ptr] << 3) | ((attr & 0x80) << 4) | ((attr & 0x08) << 10);
#else
		uint16_t code = (vram[ptr] << 3) | ((attr & 0x80) << 4);
#endif
		uint8_t col_b = attr & 7;
		uint8_t col_f = (attr >> 4) & 7;
#if defined(_MZ700)
		uint8_t pat_t = pcg_active ? pcg[code | (v & 7)] : font[code | (v & 7)];
#else
		uint8_t pat_t = font[code | (v & 7)];
#endif
		uint8_t* dest = &screen[v][x];
		
#if defined(_MZ1500)
		if((priority & 1) && (pcg_attr & 8)) {
			uint16_t pcg_code = (vram[ptr | 0x400] << 3) | ((pcg_attr & 0xc0) << 5);
			uint8_t pcg_dot[8];
			uint8_t pat_b = pcg[pcg_code | (v & 7) | 0x0000];
			uint8_t pat_r = pcg[pcg_code | (v & 7) | 0x2000];
			uint8_t pat_g = pcg[pcg_code | (v & 7) | 0x4000];
			pcg_dot[0] = ((pat_b & 0x80) >> 7) | ((pat_r & 0x80) >> 6) | ((pat_g & 0x80) >> 5);
			pcg_dot[1] = ((pat_b & 0x40) >> 6) | ((pat_r & 0x40) >> 5) | ((pat_g & 0x40) >> 4);
			pcg_dot[2] = ((pat_b & 0x20) >> 5) | ((pat_r & 0x20) >> 4) | ((pat_g & 0x20) >> 3);
			pcg_dot[3] = ((pat_b & 0x10) >> 4) | ((pat_r & 0x10) >> 3) | ((pat_g & 0x10) >> 2);
			pcg_dot[4] = ((pat_b & 0x08) >> 3) | ((pat_r & 0x08) >> 2) | ((pat_g & 0x08) >> 1);
			pcg_dot[5] = ((pat_b & 0x04) >> 2) | ((pat_r & 0x04) >> 1) | ((pat_g & 0x04) >> 0);
			pcg_dot[6] = ((pat_b & 0x02) >> 1) | ((pat_r & 0x02) >> 0) | ((pat_g & 0x02) << 1);
			pcg_dot[7] = ((pat_b & 0x01) >> 0) | ((pat_r & 0x01) << 1) | ((pat_g & 0x01) << 2);
			
			if(priority & 2) {
				// pcg > text
				dest[0] = pcg_dot[0] ? pcg_dot[0] : (pat_t & 0x80) ? col_f : col_b;
				dest[1] = pcg_dot[1] ? pcg_dot[1] : (pat_t & 0x40) ? col_f : col_b;
				dest[2] = pcg_dot[2] ? pcg_dot[2] : (pat_t & 0x20) ? col_f : col_b;
				dest[3] = pcg_dot[3] ? pcg_dot[3] : (pat_t & 0x10) ? col_f : col_b;
				dest[4] = pcg_dot[4] ? pcg_dot[4] : (pat_t & 0x08) ? col_f : col_b;
				dest[5] = pcg_dot[5] ? pcg_dot[5] : (pat_t & 0x04) ? col_f : col_b;
				dest[6] = pcg_dot[6] ? pcg_dot[6] : (pat_t & 0x02) ? col_f : col_b;
				dest[7] = pcg_dot[7] ? pcg_dot[7] : (pat_t & 0x01) ? col_f : col_b;
			} else {
				// text_fore > pcg > text_back
				dest[0] = (pat_t & 0x80) ? col_f : pcg_dot[0] ? pcg_dot[0] : col_b;
				dest[1] = (pat_t & 0x40) ? col_f : pcg_dot[1] ? pcg_dot[1] : col_b;
				dest[2] = (pat_t & 0x20) ? col_f : pcg_dot[2] ? pcg_dot[2] : col_b;
				dest[3] = (pat_t & 0x10) ? col_f : pcg_dot[3] ? pcg_dot[3] : col_b;
				dest[4] = (pat_t & 0x08) ? col_f : pcg_dot[4] ? pcg_dot[4] : col_b;
				dest[5] = (pat_t & 0x04) ? col_f : pcg_dot[5] ? pcg_dot[5] : col_b;
				dest[6] = (pat_t & 0x02) ? col_f : pcg_dot[6] ? pcg_dot[6] : col_b;
				dest[7] = (pat_t & 0x01) ? col_f : pcg_dot[7] ? pcg_dot[7] : col_b;
			}
		} else {
#endif
			// text only
			dest[0] = (pat_t & 0x80) ? col_f : col_b;
			dest[1] = (pat_t & 0x40) ? col_f : col_b;
			dest[2] = (pat_t & 0x20) ? col_f : col_b;
			dest[3] = (pat_t & 0x10) ? col_f : col_b;
			dest[4] = (pat_t & 0x08) ? col_f : col_b;
			dest[5] = (pat_t & 0x04) ? col_f : col_b;
			dest[6] = (pat_t & 0x02) ? col_f : col_b;
			dest[7] = (pat_t & 0x01) ? col_f : col_b;
#if defined(_MZ1500)
		}
#endif
		ptr++;
	}
}

void MEMORY::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// draw lines
		for(int v = 0; v < 200; v++) {
			draw_line(v);
		}
	}
	
	// copy to real screen
	emu->set_vm_screen_lines(200);
	
	for(int y = 0; y < 200; y++) {
		scrntype_t* dest0 = emu->get_screen_buffer(2 * y);
		scrntype_t* dest1 = emu->get_screen_buffer(2 * y + 1);
		uint8_t* old = screen_copy[y];
		uint8_t* src = screen[y];
		
		for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
#if defined(_MZ1500)
			dest0[x2] = dest0[x2 + 1] = palette_pc[palette[src[x] & 7]];
#else
			if (config.color_blender) {
				dest0[x2] = dest0[x2 + 1] = (palette_pc[src[x] & 7] & RGB_COLOR(0x7F, 0x7F, 0x7F)) + (palette_pc[old[x] & 7] & RGB_COLOR(0x7F, 0x7F, 0x7F));
			} else {
				dest0[x2] = dest0[x2 + 1] = (palette_pc[src[x] & 7]);
			}
#endif
		}
		if(!config.scan_line) {
			my_memcpy(dest1, dest0, 640 * sizeof(scrntype_t));
			if (config.color_blender) {
				my_memcpy(old, src, 320 * sizeof(uint8_t));
			}
		} else {
			memset(dest1, 0, 640 * sizeof(scrntype_t));
			if (config.color_blender) {
				my_memcpy(old, src, 320 * sizeof(uint8_t));
			}
		}
	}
	emu->screen_skip_line(true);
}

#define STATE_VERSION	3

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
#if defined(_MZ700)
	state_fio->StateArray(pcg + 0x400, 0x400, 1);
	state_fio->StateArray(pcg + 0xc00, 0x400, 1);
#elif defined(_MZ1500)
	state_fio->StateArray(pcg, sizeof(pcg), 1);
#endif
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateValue(mem_bank);
#if defined(_MZ700)
	state_fio->StateValue(pcg_data);
	state_fio->StateValue(pcg_addr);
	state_fio->StateValue(pcg_ctrl);
#elif defined(_MZ1500)
	state_fio->StateValue(pcg_bank);
#endif
#if defined(_MZ1500)
	state_fio->StateValue(priority);
	state_fio->StateArray(palette, sizeof(palette), 1);
#endif
	state_fio->StateValue(blink);
	state_fio->StateValue(tempo);
#if defined(_MZ700)
	state_fio->StateValue(blank);
#endif
	state_fio->StateValue(hblank);
	state_fio->StateValue(hsync);
	state_fio->StateValue(vblank);
	state_fio->StateValue(vsync);
#if defined(_MZ700) || defined(_MZ1500)
	state_fio->StateValue(blank_vram);
#endif
#if defined(_MZ1500)
	state_fio->StateValue(hblank_pcg);
#endif
	state_fio->StateArray(palette_pc, sizeof(palette_pc), 1);

#if defined(USE_ROMDISK)
	state_fio->StateValue(ipl_page);
	state_fio->StateValue(ipl_storage);
#endif

	// post process
	if(loading) {
		update_map_low();
		update_map_high();
	}
	return true;
}

