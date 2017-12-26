#ifndef __C_TYPES_H__
#define __C_TYPES_H__

#include <stdio.h>
#include <string.h>

#ifndef int8_t
typedef signed char int8_t;
#endif
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef int16_t
typedef signed short int16_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef int32_t
typedef signed int int32_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

typedef unsigned char bool;

#define ICACHE_FLASH_ATTR

#define FALSE		0
#define TRUE		1

#endif /* __C_TYPES_H__ */

