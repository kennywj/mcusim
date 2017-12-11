//
// fs_port.h
//
#include "ff.h"
#define MAX_FS_ARGS        8
// define API's ID
#define FS_OPEN         0x01
#define FS_CLOSE        0x02
#define FS_READ         0x03
#define FS_WRITE        0x04
#define FS_UNLINK       0x05
#define FS_STAT         0x06
#define FS_LSEEK        0x07
#define FS_TURNCATE     0x08
#define FS_SYNC         0x09


#define FS_OPENDIR      0x21
#define FS_CLOSEDIR     0x22
#define FS_READDIR      0x23
#define FS_FINDFIRST    0x24
#define FS_FINDNEXT     0x25
#define FS_MKDIR        0x26
#define FS_CHANGEDIR    0x27
#define FS_GETCWD       0x28

#define FS_RENAME       0x40
#define FS_GETFREE      0x41

typedef struct _fs_msg_
{
    struct _fs_msg_ *next;  //  link pointer
    xSemaphoreHandle sem;   // wait semaphore
    unsigned int id;        // API index
    int res;
    void *args[MAX_FS_ARGS]; 
}FS_MSG;

int fs_open (const char* path, BYTE mode);			/* Open or create a file */
int fs_close (int fd);											/* Close an open file object */
int fs_read (int fd, void* buff, UINT btr);			/* Read data from the file, return >=0 means real read data size */
int fs_write (int fd, const void* buff, UINT btw);	/* Write data to the file, return >=0 means real write data size */
int fs_lseek (int fd, FSIZE_t ofs);							/* Move file pointer of the file object */
int fs_truncate (int fd);										/* Truncate the file */
int fs_sync (int fd);											/* Flush cached data of the writing file */
int fs_opendir (const char* path);					/* Open a directory */
int fs_closedir (int dd);										/* Close an open directory */
int fs_readdir (int dd, FILINFO* fno);							/* Read a directory item */
int fs_findfirst (int dd, FILINFO* fno, const char* path, const char* pattern);	/* Find first file */
int fs_findnext (int dd, FILINFO* fno);							/* Find next file */
int fs_mkdir (const char* path);								/* Create a sub directory */
int fs_unlink (const char* path);								/* Delete an existing file or directory */
int fs_rename (const char* path_old, const char* path_new);	/* Rename/Move a file or directory */
int fs_stat (const char* path, FILINFO* fno);					/* Get file status */
int fs_chmod (const char* path, BYTE attr, BYTE mask);			/* Change attribute of a file/dir */
int fs_chdir (const char* path);								/* Change current directory */
int fs_getcwd (char* buff, UINT len);							/* Get current directory */
int fs_getfree (const char* path, DWORD* nclst, FATFS** fatfs);	/* Get number of free clusters on the drive */
#if 0
// has not implement
FRESULT fs_chdrive (const char* path);								/* Change current drive */
FRESULT fs_utime (const char* path, const FILINFO* fno);			/* Change timestamp of a file/dir */
FRESULT fs_getlabel (const char* path, char* label, DWORD* vsn);	/* Get volume label */
FRESULT fs_setlabel (const char* label);							/* Set volume label */
FRESULT fs_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
FRESULT fs_expand (FIL* fp, FSIZE_t szf, BYTE opt);					/* Allocate a contiguous block to the file */
FRESULT fs_mount (FATFS* fs, const char* path, BYTE opt);			/* Mount/Unmount a logical drive */
FRESULT fs_mkfs (const char* path, BYTE opt, DWORD au, void* work, UINT len);	/* Create a FAT volume */
FRESULT fs_fdisk (BYTE pdrv, const DWORD* szt, void* work);			/* Divide a physical drive into so */
#endif
