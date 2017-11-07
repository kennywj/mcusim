/**
  ******************************************************************************
  * @file    diskio_ramdisk.c
  * @author  IoT Firmware Team
  * @version V1.0.0
  * @date    25-August-2017
  * @brief   This file provides firmware to transfer data from or to disk.
  *
  *  @verbatim
 ===============================================================================
                   ##### How to use this file #####
 ===============================================================================
    [..]
    Detail about how to use this file.

   @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>Copyright &copy; 2016 FuTaiKang Electronics Development (Yantai) Ltd.</center></h2>
  * <h2><center>Copyright &copy; 2014-2015 Socle Technology Corporation.</center></h2>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include "diskio.h"		/* FatFs lower layer API */
#include "ramdisk.h"

/* Private variables ---------------------------------------------------------*/
static volatile DSTATUS Stat = STA_NOINIT;
static char *pmem=NULL;

/**
  * @brief  RAMDisk Status
  * @return DSTATUS      Operation status
  */
DRESULT RAM_disk_status(void)
{
    return Stat;
}

/**
  * @brief Initialize the RAMDISK
  * @return DSTATUS	Operation status
  */
DRESULT RAM_disk_initialize(void)
{
    Stat = STA_NOINIT;

    if (!pmem)
        pmem = malloc(SRAM_DEVICE_SIZE);
    if (pmem)    
        Stat &= ~STA_NOINIT;
    else
        Stat |= STA_NODISK;

    return Stat;
}

/*-----------------------------------------------------------------------*/
/* RAMDISK Read Sector(s)                                                */
/*-----------------------------------------------------------------------*/
/**
  * @brief Read block(s) to a specified address in RAMDISK, in polling mode.
  * @param buff: Pointer to the buffer that will contain the data to transmit
  * @param sector: Address from where data is to be read
  * @param count: Number of RAMDISK blocks to read
  * @return DRESULT	Operation status
  */
DRESULT RAM_disk_read(BYTE *buff, DWORD sector, UINT count)
{

    unsigned int BufferSize = (BLOCK_SIZE * count);
    unsigned char *pSramAddress = (unsigned char *) (pmem + (sector * BLOCK_SIZE));

    for(; BufferSize != 0; BufferSize--)
    {
        *buff++ = *( unsigned char *)pSramAddress++;
    }

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/*RAMDISK Write Sector(s)                                                */
/*-----------------------------------------------------------------------*/
/**
  * @brief Write block(s) to a specified address in RAMDISK, in polling mode.
  * @param buff: Pointer to the buffer that will contain the data to transmit
  * @param sector: Address from where data is to be read
  * @param count: Number of RAMDISK blocks to read
  * @return DRESULT	Operation status
  */
DRESULT RAM_disk_write(const BYTE *buff, DWORD sector, UINT count)
{
    unsigned int BufferSize = (BLOCK_SIZE * count);
    unsigned char *pSramAddress = (unsigned char *) (pmem + (sector * BLOCK_SIZE));

    for(; BufferSize != 0; BufferSize--)
    {
        *( unsigned char *)pSramAddress++ = *buff++;
    }

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* RAMDISK Miscellaneous Functions                                       */
/*-----------------------------------------------------------------------*/
/**
  * @brief  Control RAMDISK Data Transfer
  * @param  cmd      Command
  * @param  buff     Data buffer
  * @return DRESULT  Operation status
  */
DRESULT RAM_disk_ioctl(BYTE cmd, void *buff)
{
    DRESULT res=RES_PARERR;

    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (cmd)
    {
        /* Make sure that no pending write process */
    case CTRL_SYNC :
        res = RES_OK;
        break;

        /* Get number of sectors on the disk (DWORD) */
    case GET_SECTOR_COUNT :
        *(DWORD*)buff = SRAM_DEVICE_SIZE / BLOCK_SIZE;
        res = RES_OK;
        break;

        /* Get R/W sector size (WORD) */
    case GET_SECTOR_SIZE :
        *(WORD*)buff = BLOCK_SIZE;
        res = RES_OK;
        break;

        /* Get erase block size in unit of sector (DWORD) */
    case GET_BLOCK_SIZE :
        //    *(DWORD*)buff = BLOCK_SIZE;
        *(DWORD*)buff = 1;
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
    }

    return res;
}

