#include <cstdio>
#include <cstdlib>
#include "types.h"

extern u16 RAM[2048];
extern u16 ROM[36*1024];
extern u16 Input[0x200];
extern u16 Output[0x200];

/*TODO: 
	- warn about addresses outside memory? (only goes to 0xfff)
	- editing locations
	- memory mapped I/O-type locations (timers, etc)
*/

u16 mem_read(u16 addr)
{
	if( addr < OCTAL(2000) )
	{
		if( addr > OCTAL(1377) )
		{ //do some address manipulation for the banked-RAM area
			addr &= 0xff;
			addr |= (RAM[BB] & 7) << 8;
		}	
		return RAM[addr];  //nothing else need be done with lowmem "editing"/IO/timers here
	}
	
	if( addr < OCTAL(4000) )
	{  //banked ROM area
		addr &= OCTAL(1777);
		int rom_bank = RAM[BB] >> 10;
		if( rom_bank > OCTAL(27) && rom_bank < OCTAL(34) && (Output[7]&0x80) )
		{
			rom_bank += 8;		
		}
		return ROM[(rom_bank << 10) | addr];
	}
	
	return ROM[addr];  // banks 2&3 are fixed where they would be expected
}

// mem_write only allows writes to RAM and warns about anything else
// also, will allow 16bit writes to A and Q, but anything else gets
// MSb cleared.
void mem_write(u16 addr, u16 val)
{
	if( addr > OCTAL(1777) )
	{
		printf("Warning: attempted write to ROM @0%o = 0x%x\n", addr, val);
		return;
	}
	
	if( addr > OCTAL(1377) )
	{
		addr &= 0xff;
		addr |= (RAM[BB] & 7) << 8;
	}

	if( addr != 0 && addr != 2 )
	{  // A and Q are a full 16bits, anything else needs masking
		val &= 0x7fff;
	}
	
	switch( addr )
	{
	case OCTAL(20): val = (val<<15)|(val>>1); break;
	case OCTAL(21): val = (val>>1)|(val&0x8000); break;
	case OCTAL(22): val = (val<<1)|(val>>15); break;
	case OCTAL(23): val = (val>>7)&0x7f; break;
	default: break;  // there's a bunch of memory-mapped IO type things that will be handled in other places.
	}
	
	RAM[addr] = val;
	return;
}

void io_write(u16 addr, u16 val)
{

	return;
}

u16 io_read(u16 addr)
{

	return 0;
}


