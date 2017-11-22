//
//porting of fatfs
//
#include <stdio.h> 
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>     /* memcpy */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "ff.h"		    /* FatFs lower layer API */
#include "diskio.h"		/* FatFs lower layer API */
#include "fs_port.h"	/* porting of file system */

#define MAX_FSCMD   4

static int filesystem_inited = 0;
static xQueueHandle fsrcvq;
static xTaskHandle hFsTask;
static FS_MSG *msg_list;
//
// message block list operation
//  get/free message block
//
static FS_MSG *get_msgblk(void)
{   
    FS_MSG *msg;
    
    msg = msg_list;
    if (msg)
        msg_list = msg->next;
    return msg;        
}

static void free_msgblk(FS_MSG *msg)
{
    msg->next = msg_list;
    msg_list = msg;
}

//
//  function: do_fs
//      intiial RAM¡@file system
//  parameters
//      none
//
void do_fs(void *parm)
{
    FATFS fs,*pfs;
    FS_MSG *msg;
    BYTE work[_MAX_SS]; /* Work area (larger is better for processing time) */
    int res;
    DWORD fre_clust, fre_sect, tot_sect;
    
    res = f_mkfs("", FM_ANY, 0, work, sizeof work);
    if (res != FR_OK)
    {
        printf("make ram file system fail %d\n",res);
        goto end_do_fs;
    }
    /* Register work area */
    res = f_mount(&fs, "", 0);
    if (res != FR_OK)
    {
        printf("mount disk fail %d\n",res);
        goto end_do_fs;
    }
    /* Get volume information and free clusters of drive 1 */
    res = f_getfree("", &fre_clust, &pfs);
    if (res)
     {
        printf("get free space fail %d\n",res);
        goto end_do_fs;
    }
    /* Get total sectors and free sectors */
    tot_sect = (pfs->n_fatent - 2) * pfs->csize;
    fre_sect = fre_clust * pfs->csize;

    /* Print the free space (assuming 512 bytes/sector) */
    printf("\nRAM disk ready. %10lu KiB total drive space. %10lu KiB available.\n",
           tot_sect / 2, fre_sect / 2);
    while(1)
    {
        if ( pdTRUE != xQueueReceive( fsrcvq, ( void * )&msg, portMAX_DELAY ) )
        {
            printf("%s: receive message error\n",__FUNCTION__);  
            break;
        }    
        switch(msg->id)
        {
            case FS_OPEN:
                msg->res = f_open((FIL *)msg->args[0], (const char*)msg->args[1], *(BYTE *)msg->args[2]);
            break;
            case FS_CLOSE:
                msg->res = f_close((FIL *)msg->args[0]);
            break;
            case FS_READ:
                msg->res = f_read((FIL *)msg->args[0], (const char*)msg->args[1], (UINT)msg->args[2], &res);
                msg->args[2] = res; // read bytes
            break;
            case FS_WRITE:
                msg->res = f_write((FIL *)msg->args[0], (const char*)msg->args[1], *(BYTE *)msg->args[2], &res);
                msg->args[2] = res; // write bytes
            break;
            case FS_UNLINK:
                msg->res = f_unlink((char*)msg->args[0]);
            break;
            case FS_LSEEK:
                msg->res = f_lseek((FIL*)msg->args[0],(FSIZE_t)msg->args[1]);
            break;
            case FS_TURNCATE:
                msg->res = f_truncate((FIL*)msg->args[0]);
            break;
            case FS_SYNC:
                msg->res = f_sync((FIL*)msg->args[0]);
            break;
            case FS_OPENDIR: 
                msg->res = f_opendir((DIR* )msg->args[0], (const char*)msg->args[1]);
            break;
            case FS_CLOSEDIR:
                msg->res = f_closedir((DIR* )msg->args[0]);
            break;
            case FS_READDIR:
                msg->res = f_readdir((DIR* )msg->args[0], (FILINFO *)msg->args[1]);
            break;
            case FS_FINDFIRST:
                msg->res = f_findfirst((DIR*)msg->args[0],(FILINFO*)msg->args[1],
                    (const TCHAR*)msg->args[2],(const TCHAR*)msg->args[3]);
            break;
            case FS_FINDNEXT:
                msg->res = f_findnext((DIR*)msg->args[0],(FILINFO*)msg->args[1]);
            break;
            case FS_MKDIR:
                msg->res = f_mkdir((const TCHAR*)msg->args[0]);
            break;
            case FS_GETFREE:
                pfs = (FATFS *)msg->args[1];
                msg->res = f_getfree((char*)msg->args[0], &fre_clust, &pfs);
                if (msg->res == FR_OK)
                {
                    msg->args[1] = (void *)pfs;
                    *(unsigned int *)&msg->args[2] = fre_clust;
                }
            break;
            case FS_CHANGEDIR:
                msg->res = f_chdir((char*)msg->args[0]);
            break;
            case FS_GETCWD:
                msg->res = f_getcwd((char*)msg->args[0],(unsigned int)msg->args[1]);
            break;
            case FS_RENAME:
                msg->res = f_rename((char*)msg->args[0],(char *)msg->args[1]);
            break;
            case FS_STAT:
                msg->res = f_stat((char*)msg->args[0],(FILINFO*)msg->args[1]);
            break;
            default:
                msg->res = FR_INT_ERR;
            break;
        }
        xSemaphoreGive(msg->sem);
    }
end_do_fs:
    printf("exit fs service\n");  
    while((msg=get_msgblk())!=NULL)
    {
        //vSemaphoreDelete(msg->sem);
        free(msg);
    }
    vTaskEndScheduler();    
}

//
//  function: fs_init
//      create file system thread
//  parameters
//      none
//
void fs_init()
{
    int i;
    FS_MSG *msg;
    // create message buffers
    for (i=0;i<MAX_FSCMD;i++)
    {
        msg = malloc(sizeof(FS_MSG));
        if (!msg)
        {
            printf("create fs message block fail\n");
            goto end_fs_init;
        }
        memset(msg,0,sizeof(FS_MSG));
        vSemaphoreCreateBinary(msg->sem);
        xSemaphoreTake(msg->sem, portMAX_DELAY);
        free_msgblk(msg);
    }
    
    // create a receive queue
    fsrcvq = xQueueCreate( MAX_FSCMD, sizeof ( void * ) );
    if (fsrcvq==NULL)
    {
        printf("create recv queue fail\n");
        goto end_fs_init;
    }
    // create a filesystem thread
   	xTaskCreate( do_fs, "fs", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &hFsTask );
end_fs_init:
    return;
}

/* Open or create a file */
FRESULT fs_open (FIL* fp, const char* path, BYTE mode)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_OPEN;
        msg->args[0] = (void *)fp;
        msg->args[1] = (void *)path;
        msg->args[2] = (void *)&mode;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
                res = msg->res;
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Close an open file object */
FRESULT fs_close (FIL* fp)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_CLOSE;
        msg->args[0] = (void *)fp;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
                res = msg->res;
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}									

/* Read data from the file */
FRESULT fs_read (FIL* fp, void* buff, UINT btr, UINT* br)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_READ;
        msg->args[0] = (void *)fp;
        msg->args[1] = (void *)buff;
        msg->args[2] = (void *)btr;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
                *br = msg->args[2];
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Write data to the file */
FRESULT fs_write (FIL* fp, const void* buff, UINT btw, UINT* bw)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_WRITE;
        msg->args[0] = (void *)fp;
        msg->args[1] = (void *)buff;
        msg->args[2] = (void *)&btw;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
                *bw = msg->args[2]; // real write data bytes
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Open a directory */
FRESULT fs_opendir (DIR* dp, const char* path)			
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_OPENDIR;
        msg->args[0] = (void *)dp;
        msg->args[1] = (void *)path;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
                res = msg->res;
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Close an open directory */	
FRESULT fs_closedir (DIR* dp)										
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_CLOSEDIR;
        msg->args[0] = (void *)dp;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
                res = msg->res;
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Read a directory item */
FRESULT fs_readdir (DIR* dp, FILINFO* fno)						
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_READDIR;
        msg->args[0] = (void *)dp;
        msg->args[1] = (void *)fno;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
                res = msg->res;
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Get number of free clusters on the drive */
FRESULT fs_getfree (const char* path, DWORD* nclst, FATFS** fatfs)	
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_GETFREE;
        msg->args[0] = (void *)path;
        msg->args[1] = (void *)(*fatfs);
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                *fatfs = (FATFS*)msg->args[1];
                *nclst = (DWORD)msg->args[2];
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Delete an existing file or directory */
FRESULT fs_unlink (const char* path)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_UNLINK;
        msg->args[0] = (void *)path;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}		    
 
/* Get file status */
FRESULT fs_stat (const char* path, FILINFO* fno)					
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_STAT;
        msg->args[0] = (void *)path;
        msg->args[1] = (void *)fno;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}
			
/* Find first file */
FRESULT fs_findfirst (DIR* dp, FILINFO* fno, const char* path, const char* pattern)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_FINDFIRST;
        msg->args[0] = (void *)dp;
        msg->args[1] = (void *)fno;
        msg->args[2] = (void *)path;
        msg->args[3] = (void *)pattern;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}
/* Find next file */
FRESULT fs_findnext (DIR* dp, FILINFO* fno)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_FINDNEXT;
        msg->args[0] = (void *)dp;
        msg->args[1] = (void *)fno;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Create a sub directory */
FRESULT fs_mkdir (const char* path)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_MKDIR;
        msg->args[0] = (void *)path;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}
/* Change current directory */
FRESULT fs_chdir (const char* path)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_CHANGEDIR;
        msg->args[0] = (void *)path;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}								
/* Get current directory */
FRESULT fs_getcwd (char* buff, UINT len)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_GETCWD;
        msg->args[0] = (void *)buff;
        msg->args[1] = (void *)len;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Rename/Move a file or directory */
FRESULT fs_rename (const char* path_old, const char* path_new)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_RENAME;
        msg->args[0] = (void *)path_old;
        msg->args[1] = (void *)path_new;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}
/* Move file pointer of the file object */
FRESULT fs_lseek (FIL* fp, FSIZE_t ofs)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_LSEEK;
        msg->args[0] = (void *)fp;
        msg->args[1] = (FSIZE_t)ofs;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}							

/* Truncate the file */
FRESULT fs_truncate (FIL* fp)
{
     int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_TURNCATE;
        msg->args[0] = (void *)fp;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}

/* Flush cached data of the writing file */
FRESULT fs_sync (FIL* fp)
{
    int res=FR_OK;
    FS_MSG *msg;
    
    msg=get_msgblk();
    if(msg)
    {
        msg->id = FS_SYNC;
        msg->args[0] = (void *)fp;
        if( xQueueSend( fsrcvq,
                       ( void * ) &msg,
                       ( portTickType ) 10 ) == pdPASS )
        {
            // wait fs thread complete the request
            if (pdTRUE == xSemaphoreTake(msg->sem, portMAX_DELAY))
            {
                res = msg->res;
            }
            else
                res = FR_NOT_READY;
        }
        else    /* Failed to post the message, even after 10 ticks. */
            res = FR_NOT_READY;
        free_msgblk(msg);
    }
    else
        res = FR_NOT_READY;
    return res;
}						

#if 0

FRESULT fs_chmod (const char* path, BYTE attr, BYTE mask);			/* Change attribute of a file/dir */
FRESULT fs_utime (const char* path, const FILINFO* fno);			/* Change timestamp of a file/dir */
FRESULT fs_chdrive (const char* path);								/* Change current drive */
FRESULT fs_getlabel (const char* path, char* label, DWORD* vsn);	/* Get volume label */
FRESULT fs_setlabel (const char* label);							/* Set volume label */
FRESULT fs_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
FRESULT fs_expand (FIL* fp, FSIZE_t szf, BYTE opt);					/* Allocate a contiguous block to the file */
FRESULT fs_mount (FATFS* fs, const char* path, BYTE opt);			/* Mount/Unmount a logical drive */
FRESULT fs_mkfs (const char* path, BYTE opt, DWORD au, void* work, UINT len);	/* Create a FAT volume */
FRESULT fs_fdisk (BYTE pdrv, const DWORD* szt, void* work);	
#endif