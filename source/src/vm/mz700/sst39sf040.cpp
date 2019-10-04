/*
	Author : Christophe.Avoinne
	Date   : 2019.10.01 -

	[ 512KB nor-flash memory ]
*/

#include "sst39sf040.h"

#define DATA_SIZE	128*4096
#define ADDR_MASK	(DATA_SIZE - 1)

void SST39SF040::initialize()
{
	// init memory
	data_buffer = (uint8_t *)malloc(DATA_SIZE);
	memset(data_buffer, 255, DATA_SIZE);
	modified = false;
	wc = WC1_XXXXYY;
	software_id_entry = false;
	busy = 0;
	byte = 0x40;

	// load ds1249y image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("SST39SF040.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(data_buffer, DATA_SIZE, 1);
		fio->Fclose();
	}
	delete fio;
}

void SST39SF040::release()
{
	// save ds1249y image
	if(modified) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("SST39SF040.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(data_buffer, DATA_SIZE, 1);
			fio->Fclose();
		}
		delete fio;
	}
	
	// release memory
	free(data_buffer);
}

void SST39SF040::reset()
{
	wc = WC1_XXXXYY;
	software_id_entry = false;
	busy = 0;
	byte = 0xFF;
}

void SST39SF040::write_data8(uint32_t addr, uint32_t data)
{
	if (busy) {
		return;
	}
	uint32_t code = (addr << 8) | (data & 255);
	switch (wc) {
	case WC1_XXXXYY:
		if (code == 0x5555AA) {
			wc = WC1_5555AA;
		}
		else if (software_id_entry && (data == 0xF0)) {
			software_id_entry = false;
		}
		break;

	case WC1_5555AA:
		if (code == 0x2AAA55) {
			wc = WC2_2AAA55;
			break;
		} 
		wc = WC1_XXXXYY;
		break;

	case WC2_2AAA55:
		switch (code) {
		case 0x555580:
			wc = WC3_555580;
			break;
		case 0x555590:
			software_id_entry = true;
			wc = WC1_XXXXYY;
			break;
		case 0x5555A0:
			wc = WC3_5555A0;
			break;
		default:
			wc = WC1_XXXXYY;
			break;
		}

	case WC3_555580:
		if (code == 0x5555AA) {
			wc = WC4_5555AA;
			break;
		}		
		wc = WC1_XXXXYY;
		break;

	case WC3_555590:
		if (code == 0x5555AA) {
			wc = WC4_5555AA;
			break;
		}
		else if (data == 0xF0) {
			software_id_entry = false;
			wc = WC1_XXXXYY;
			break;
		}
		break;

	case WC3_5555A0:
		modified = true;
		data_buffer[addr & ADDR_MASK] = uint8_t(data);
		busy = 4;
		wc = WC1_XXXXYY;
		break;

	case WC4_5555AA:
		if (code == 0x2AAA55) {
			wc = WC5_2AAA55;
			break;
		}
		wc = WC1_XXXXYY;
		break;

	case WC5_2AAA55:
		if (code == 0x555510) {
			busy = 20000;
			memset(data_buffer, 0xFF, DATA_SIZE);
			modified = true;
			wc = WC1_XXXXYY;
			break;
		}
		else if (data == 0x30) {
			busy = 5000;
			memset(data_buffer+(addr & (ADDR_MASK ^ 0x0FFF)), 0xFF, 0x1000);
			modified = true;
			wc = WC1_XXXXYY;
			break;
		}
		wc = WC1_XXXXYY;
		break;
	}
}

uint32_t SST39SF040::read_data8(uint32_t addr)
{
	uint32_t byte = data_buffer[addr & ADDR_MASK];
	if (busy) {
		--busy;
		return ((byte ^ 0x40) | 0x80) ^ ((busy & 1) << 7);
	}
	if (software_id_entry) {
		switch (addr) {
		case 0:
			return 0xBF;
		case 1:
			return 0xB7;
		default:
			return 0xff; 
		}
	}
	return uint32_t(data_buffer[addr & ADDR_MASK]);
}

#define STATE_VERSION	1

bool SST39SF040::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(data_buffer, DATA_SIZE, 1);
	state_fio->StateValue(modified);
	state_fio->StateValue(software_id_entry);
	state_fio->StateValue(busy);
	return true;
}

