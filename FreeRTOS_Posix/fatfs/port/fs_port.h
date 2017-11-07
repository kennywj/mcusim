//
// fs_port.h
//
#include "ff.h"

#define MAX_ARGS    8
// define API's ID
#define FS_OPEN     0
#define FS_CLOSE    1
#define FS_READ     2
#define FS_WRITE    3
#define FS_OPENDIR  4
#define FS_CLOSEDIR 5
#define FS_READDIR  6
#define FS_GETFREE  7
#define FS_UNLINK   8

typedef struct _fs_msg_
{
    struct _fs_msg_ *next;  //  link pointer
    xSemaphoreHandle sem;   // wait semaphore
    unsigned int id;        // API index
    int res;
    void *args[MAX_ARGS]; 
}FS_MSG;

FRESULT fs_open (FIL* fp, const char* path, BYTE mode);			/* Open or create a file */
FRESULT fs_close (FIL* fp);											/* Close an open file object */
FRESULT fs_read (FIL* fp, void* buff, UINT btr, UINT* br);			/* Read data from the file */
FRESULT fs_write (FIL* fp, const void* buff, UINT btw, UINT* bw);	/* Write data to the file */
FRESULT fs_lseek (FIL* fp, FSIZE_t ofs);							/* Move file pointer of the file object */
FRESULT fs_truncate (FIL* fp);										/* Truncate the file */
FRESULT fs_sync (FIL* fp);											/* Flush cached data of the writing file */
FRESULT fs_opendir (DIR* dp, const char* path);					/* Open a directory */
FRESULT fs_closedir (DIR* dp);										/* Close an open directory */
FRESULT fs_readdir (DIR* dp, FILINFO* fno);							/* Read a directory item */
FRESULT fs_findfirst (DIR* dp, FILINFO* fno, const char* path, const char* pattern);	/* Find first file */
FRESULT fs_findnext (DIR* dp, FILINFO* fno);							/* Find next file */
FRESULT fs_mkdir (const char* path);								/* Create a sub directory */
FRESULT fs_unlink (const char* path);								/* Delete an existing file or directory */
FRESULT fs_rename (const char* path_old, const char* path_new);	/* Rename/Move a file or directory */
FRESULT fs_stat (const char* path, FILINFO* fno);					/* Get file status */
FRESULT fs_chmod (const char* path, BYTE attr, BYTE mask);			/* Change attribute of a file/dir */
FRESULT fs_utime (const char* path, const FILINFO* fno);			/* Change timestamp of a file/dir */
FRESULT fs_chdir (const char* path);								/* Change current directory */
FRESULT fs_chdrive (const char* path);								/* Change current drive */
FRESULT fs_getcwd (char* buff, UINT len);							/* Get current directory */
FRESULT fs_getfree (const char* path, DWORD* nclst, FATFS** fatfs);	/* Get number of free clusters on the drive */
FRESULT fs_getlabel (const char* path, char* label, DWORD* vsn);	/* Get volume label */
FRESULT fs_setlabel (const char* label);							/* Set volume label */
FRESULT fs_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
FRESULT fs_expand (FIL* fp, FSIZE_t szf, BYTE opt);					/* Allocate a contiguous block to the file */
FRESULT fs_mount (FATFS* fs, const char* path, BYTE opt);			/* Mount/Unmount a logical drive */
FRESULT fs_mkfs (const char* path, BYTE opt, DWORD au, void* work, UINT len);	/* Create a FAT volume */
FRESULT fs_fdisk (BYTE pdrv, const DWORD* szt, void* work);			/* Divide a physical drive into so */