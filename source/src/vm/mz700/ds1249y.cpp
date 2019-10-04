/*
	Author : Christophe.Avoinne
	Date   : 2019.10.01 -

	[ 256KB nvram memory ]
*/

#include "ds1249y.h"

#define DATA_SIZE	64*4096
#define ADDR_MASK	(DATA_SIZE - 1)

void DS1249Y::initialize()
{
	// init memory
	data_buffer = (uint8_t *)malloc(DATA_SIZE);
	memset(data_buffer, 0, DATA_SIZE);
	modified = false;
	
	// load ds1249y image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("DS1249Y.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(data_buffer, DATA_SIZE, 1);
		fio->Fclose();
	}
	delete fio;
}

void DS1249Y::release()
{
	// save ds1249y image
	if(modified) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("DS1249Y.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(data_buffer, DATA_SIZE, 1);
			fio->Fclose();
		}
		delete fio;
	}
	
	// release memory
	free(data_buffer);
}

void DS1249Y::reset()
{
}

void DS1249Y::write_data8(uint32_t addr, uint32_t data)
{
	modified = true;
	data_buffer[addr & ADDR_MASK] = uint8_t(data);
}

uint32_t DS1249Y::read_data8(uint32_t addr)
{
	return uint32_t(data_buffer[addr & ADDR_MASK]);
}

#define STATE_VERSION	1

bool DS1249Y::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(data_buffer, DATA_SIZE, 1);
	state_fio->StateValue(modified);
	return true;
}

