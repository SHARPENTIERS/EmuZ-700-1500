/*
	Author : Christophe.Avoinne
	Date   : 2019.10.01 -

	[ 256KB nvram memory ]
*/

#ifndef _DS1249Y_H_
#define _DS1249Y_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class DS1249Y : public DEVICE
{
private:
	uint8_t *data_buffer;
	bool modified;
	
public:
	DS1249Y(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("DS1249Y"));
	}
	~DS1249Y() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

