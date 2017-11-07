/**
  ******************************************************************************
  * @file    diskio_ramdisk.h
  * @author  IoT Firmware Team
  * @version V1.0.0
  * @date    25-August-2017
  * @brief   Header file for diskio_sdio.c
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

#define BLOCK_SIZE                512
#define SRAM_DEVICE_SIZE 	  0x200000

/* Private function prototypes -----------------------------------------------*/
DRESULT RAM_disk_status(void);
DRESULT RAM_disk_initialize(void);
DRESULT RAM_disk_read(BYTE *buff, DWORD sector, UINT count);
DRESULT RAM_disk_write(const BYTE *buff, DWORD sector, UINT count);
DRESULT RAM_disk_ioctl(BYTE cmd, void *buff);

