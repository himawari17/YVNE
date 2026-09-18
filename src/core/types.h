#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef uint8_t  	u8;  //255
typedef uint16_t 	u16; //65535
typedef uint32_t 	u32; //4294967295
typedef uint64_t 	u64; // 2**64 - 1
						
typedef struct{
  u16 size;
  char *str;
} string;

#endif
