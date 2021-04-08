#include <cstdio>
#include <cstdlib>
#include "types.h"

u16 mem_read(u16 addr);
void mem_write(u16, u16);

u16 RAM[2048];
u16 ROM[36*1024];
u16 Input[0x200];
u16 Output[0x200];

#define OP_INDEX OCTAL(50000)
#define OP_EXTEND OCTAL(00006)

#define INSTR_INDEXED (B & 0x8000)

bool extend = false;
bool interrupts_enabled = false;
u16 B = 0;  // the hidden place that INDEX puts things, saved into actual RAM (BRUPT) on interrupt

u16 overflow_correct(u16);
bool IsA(u16);
bool IsQ(u16);
bool IsL(u16);
bool Is16(u16);
u16 signext(u16);
u16 add16(u16,u16);

void cpu_step()
{
	u32 temp = 0;
	//TODO: handle interrupts (and unprogrammed sequences?) here

	u16 op = B & 0x7fff;   // B contains the next instruction opcode to run, which may have been altered by multiple INDEX
	RAM[Z]++;     // Z should always contain the address of the next instruction
	u16 addr10 = OCTAL_MASK(op, 1777);  // instructions use either 10 or 12 bit addresses, or 9bit I/O space
	u16 addr12 = OCTAL_MASK(op, 7777);  // just make things cleaner by getting all ready
	u16 kc9 = OCTAL_MASK(op, 777);
	
	if( op == OCTAL(50017) && !INSTR_INDEXED && !extend )
	{  // RESUME
		interrupts_enabled = true;
		RAM[Z] = RAM[ZRUPT];
		B = RAM[BRUPT];
		return; // jumping to the end would create undesired indexing.
	}
	
	if( (op == 3 || op == 4 || op == 6) && !INSTR_INDEXED && !extend )
	{  // special instructions made from what would be TC K if it were indexed
		switch( op )
		{
		case 3: // RELINT
			interrupts_enabled = true;
			break;
		case 4: // INHINT
			interrupts_enabled = false;
			break;
		case 6: // EXTEND
			extend = true;
			break;		
		}
		B = 0;
		goto end_of_instruction;
	}
		
	if( extend )
	{
		if( OCTAL_MASK(op, 70000) == OP_INDEX )
		{
			B = mem_read(OCTAL_MASK(op, 7777));
			//extend = false;  //TODO: does INDEX clear EXTEND or not?
			goto end_of_instruction;
		} else {
			B = 0;
		}
		
		switch( OCTAL_MASK(op, 70000) )
		{
		case 0: 
			switch( OCTAL_MASK(op, 07000) )
			{  // instructions that work on the I/O channels
			case 0: break;
			case OCTAL(01000): break;
			case OCTAL(02000): break;
			case OCTAL(03000): break;			
			case OCTAL(04000): break;			
			case OCTAL(05000): break;			
			case OCTAL(06000): break;			
			case OCTAL(07000): break;
			}		
			break;
		case OCTAL(10000): 
			if( OCTAL_MASK(addr12, 6000) )
			{ // BZF
				if( RAM[A] == 0 || RAM[A] == 0xffff ) RAM[Z] = addr12;		
			} else {
				// DV todo			
			}
			break;
		case OCTAL(20000): break;
		case OCTAL(30000): // DCA K todo		
			break;
		case OCTAL(40000): // DCS K todo
			break;
		case OCTAL(50000): // technically INDEX, but should never get here.
			printf("Fatal error: INDEX should have been handled already\n"); exit(1); 
			break;
		case OCTAL(60000): 
			if( OCTAL_MASK(addr12, 6000) )
			{ // BZMF
				if( RAM[A] == 0 || (RAM[A] & 0x8000) ) RAM[Z] = addr12;			
			} else {
				// SU
				temp = mem_read(addr10);
				mem_write(addr10, temp);
				RAM[A] = sub16(RAM[A], Is16(addr10) ? temp : signext(temp));
			}	
			break;
		case OCTAL(70000): break;
		default: printf("Invalid instruction 0%o @ Z=0x%x\n", op, RAM[Z]); exit(1);
		} //end of extended instruction switch
		
		extend = false;
	} else {
		if( OCTAL_MASK(op, 76000) == OP_INDEX )
		{
			B = mem_read(OCTAL_MASK(op, 1777));
			goto end_of_instruction;
		} else {
			B = 0;
		}
	
		switch( OCTAL_MASK(op, 70000) )
		{
		case 0: // TC K
			B = mem_read(addr12);
			RAM[Q] = RAM[Z];
			RAM[Z] = addr12;
			return; // we don't want to get to the end B twiddling
		case OCTAL(10000): 
			if( OCTAL_MASK(op, 6000) )
			{ //TCF K
				RAM[Z] = addr12;			
			} else {
				//CCS K todo				
			}		
			break;
		case OCTAL(20000): break;
		case OCTAL(30000): //CA A
			temp = mem_read(addr12);
			mem_write(addr12, temp);
			RAM[A] = Is16(addr12) ? temp : signext(temp);		
			break;
		case OCTAL(40000): //CS A
			temp = mem_read(addr12);
			mem_write(addr12, temp);
			RAM[A] = ~(Is16(addr12) ? temp : signext(temp));
			break;
		case OCTAL(50000): break;
		case OCTAL(60000): //AD A
			temp = mem_read(addr12);
			mem_write(addr12, temp);
			RAM[A] = add16(RAM[A], IsA(addr12) ? temp : signext(temp));
			break;
		case OCTAL(70000): //MASK A
			temp = mem_read(addr12);
			if( Is16(addr12) )
			{
				RAM[A] &= temp;
			} else {
				RAM[A] = signext( temp & overflow_correct(RAM[A]) );
			}
			break;
		default: printf("Invalid instruction 0%o @ Z=0x%x\n", op, RAM[Z]); exit(1);		
		}//end of regular instruction switch
	}

end_of_instruction:
	B = (B ? 0x8000 : 0) | ((B + mem_read(RAM[Z])) & 0x7fff); // most of the time, B has been cleared so this is just
				     // the next instruction, but if one or more INDEX instructions
				     // have executed this will contain the accumulated alteration
				     // and setting the high bit will disallow TC K & INDEX 017 from being interpretted
				     // as INHINT, RESUME, etc.
	return;
}

u16 add16(u16 a, u16 b)
{
	u32 c = a;
	c += b;
	if( c & 0x10000 )
	{
		c++;
	}
	
	return (u16)c;
}

u16 sub16(u16 a, u16 b)
{
	u32 c = a;
	c -= b;
	if( c & 0x10000 )
	{
		c--;
	}

	return (u16)c;
}

u16 overflow_correct(u16 val)
{
	if( ((val<<1) ^ val) & 0x8000 )
	{
		val &= 0xBFFF;
		val |= (val&0x8000)>>1;
	}
	
	return val;
}

u16 signext(u16 val)
{
	val &= 0x7fff;
	val |= (val & 0x4000) << 1;
	return val;
}

bool IsA(u16 addr)
{
	if( addr > OCTAL(1777) ) return false;
	
	if( addr > OCTAL(1377) )
	{
		addr &= 0xff;
		addr |= (RAM[BB] & 7) << 8;
	}
	return (addr == 0); // is it really necessary to check for access via other banks?
}

bool IsQ(u16 addr)
{
	if( addr > OCTAL(1777) ) return false;
	
	if( addr > OCTAL(1377) )
	{
		addr &= 0xff;
		addr |= (RAM[BB] & 7) << 8;
	}
	return (addr == 2); // is it really necessary to check for access via other banks?
}

bool Is16(u16 addr)
{
	if( addr > OCTAL(1777) ) return false;

	if( addr > OCTAL(1377) )
	{
		addr &= 0xff;
		addr |= (RAM[BB] & 7) << 8;
	}
	
	return ( addr == 0 || addr == 2 );
}





