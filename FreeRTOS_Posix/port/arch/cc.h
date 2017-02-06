#ifndef __CC_H__ 
#define __CC_H__ 

#include <stdint.h>

/* Types based on stdint.h */
typedef uint8_t            u8_t; 
typedef int8_t             s8_t; 
typedef uint16_t           u16_t; 
typedef int16_t            s16_t; 
typedef uint32_t           u32_t; 
typedef int32_t            s32_t; 
typedef uintptr_t          mem_ptr_t; 
 
/* Define (sn)printf formatters for these lwIP types */
#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"
#define SZT_F "uz"
 

#define BYTE_ORDER LITTLE_ENDIAN
 
/* Use LWIP error codes */
#define LWIP_PROVIDE_ERRNO


#if defined(__arm__) && defined(__ARMCC_VERSION) 
/* Keil uVision4 tools */
#define PACK_STRUCT_BEGIN __packed
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(fld) fld
#define ALIGNED(n)  __align(n)

#endif

#endif
//__CC_H__