#include "arm_math.h"

#if !defined (ANDES_INTRINSIC)
int __SSAT(int val, unsigned int sat)
{
    int posMax, negMin;
    unsigned int i;

    posMax = 1;
    for (i = 0; i < (sat - 1); i++)
        posMax = posMax * 2;

    if (val > 0) {
      posMax = (posMax - 1);

      if (val > posMax)
        val = posMax;
    }
    else {
      negMin = -posMax;
      if (val < negMin)
         val = negMin;
    }
    return (val);
}

unsigned int __USAT(int val, unsigned int sat)
{
    unsigned int posMax;
    unsigned int i;

    posMax = 1;
    for (i = 0; i < (sat - 1); i++)
        posMax = posMax * 2;
    posMax -= 1;
    if (val > posMax)
        val = posMax;
    return val;
}

unsigned int __ROR(unsigned int val, unsigned int shift)
{
        return (((val)>>(shift)) | ((val)<<(32-(shift))));
}

unsigned int __SXTB16(unsigned int val)
{
    unsigned char c1, c2;
    unsigned short s1, s2;

    c1 = val & 0x000000ff;
    c2 = (val & 0x00ff0000) >> 16;

    int mask = 0x00000080;

    if (mask & c1)
        s1 = c1 + 0xFF00;
    else
        s1 = c1;
    if (mask & c2)
        s2 = c2 + 0xFF00;
    else
        s2 = c2;
    return ((s2<<16)|s1);

}
#endif /* ANDES_INTRINSIC */

#if defined (ARM_MATH_DSP)
/*
 * @brief C custom defined QADD16 for M3 and M0 processors
 */
__STATIC_INLINE uint32_t __QADD16(uint32_t x, uint32_t y)
{
/*  q31_t r,     s;  without initialisation 'arm_offset_q15 test' fails  but 'intrinsic' tests pass! for armCC */
  q31_t r = 0, s = 0;

  r = __SSAT(((((q31_t)x << 16) >> 16) + (((q31_t)y << 16) >> 16)), 16) & (int32_t)0x0000FFFF;
  s = __SSAT(((((q31_t)x      ) >> 16) + (((q31_t)y      ) >> 16)), 16) & (int32_t)0x0000FFFF;

  return ((uint32_t)((s << 16) | (r      )));
}

/*
 * @brief C custom defined QSUB16 for M3 and M0 processors
 */
__STATIC_INLINE uint32_t __QSUB16(uint32_t x, uint32_t y)
{
  q31_t r, s;

  r = __SSAT(((((q31_t)x << 16) >> 16) - (((q31_t)y << 16) >> 16)), 16) & (int32_t)0x0000FFFF;
  s = __SSAT(((((q31_t)x      ) >> 16) - (((q31_t)y      ) >> 16)), 16) & (int32_t)0x0000FFFF;

  return ((uint32_t)((s << 16) | (r      )));
}

/*
 * @brief C custom defined QSUB8 for M3 and M0 processors
 */
__STATIC_INLINE uint32_t __QSUB8(uint32_t x, uint32_t y)
{
  q31_t r, s, t, u;

  r = __SSAT(((((q31_t)x << 24) >> 24) - (((q31_t)y << 24) >> 24)), 8) & (int32_t)0x000000FF;
  s = __SSAT(((((q31_t)x << 16) >> 24) - (((q31_t)y << 16) >> 24)), 8) & (int32_t)0x000000FF;
  t = __SSAT(((((q31_t)x <<  8) >> 24) - (((q31_t)y <<  8) >> 24)), 8) & (int32_t)0x000000FF;
  u = __SSAT(((((q31_t)x      ) >> 24) - (((q31_t)y      ) >> 24)), 8) & (int32_t)0x000000FF;

  return ((uint32_t)((u << 24) | (t << 16) | (s <<  8) | (r      )));
}

/*
 * @brief C custom defined SMLAD for M3 and M0 processors
 */
__STATIC_INLINE uint32_t __SMLAD(uint32_t x, uint32_t y, uint32_t sum)
{
  return ((uint32_t)(((((q31_t)x << 16) >> 16) * (((q31_t)y << 16) >> 16)) +
                     ((((q31_t)x      ) >> 16) * (((q31_t)y      ) >> 16)) +
                     ( ((q31_t)sum    )                                  )   ));
}
#endif /* ARM_MATH_DSP */

