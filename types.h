#pragma once
#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define A  0
#define L  1
#define Q  2
#define EB 3
#define FB 4
#define Z  5
#define BB 6

#define ZRUPT 13 
#define BRUPT 15
// ZRUPT is Z (PC) auto-saved on interrupt
// BRUPT is the next instruction opcode to execute on resume from interrupt

#define OCTAL(a) (0##a)
#define OCTAL_MASK(a, b)  ((a) & OCTAL(b)) 


