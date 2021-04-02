#include <cstdio>
#include <iostream>
#include "types.h"

extern u16 ROM[36*1024];

template<typename I>
void printBits(I val)
{
	const int num_bits = sizeof(val) * 8;
	std::string temp; temp.resize(num_bits);
	for(int i = 0; i < num_bits; ++i)
	{
		temp[num_bits - i - 1] = ((val>>i)&1) ? '1' : '0';
	}

	std::cout << temp << '\n';
	return;
}

int main(int argc, char** args)
{
	if( argc < 2 )
	{
		printf("Usage: agc in.rom\n");
		return 1;
	}
	
	FILE* fp = fopen(args[1], "rb");
	if( ! fp )
	{
		printf("Error: Unable to open '%s'\n", args[1]);
		return 1;
	}
	[[maybe_unused]] int unu = fread(ROM, 1, 36*1024, fp);
	fclose(fp);
	
	

	return 0;
}

