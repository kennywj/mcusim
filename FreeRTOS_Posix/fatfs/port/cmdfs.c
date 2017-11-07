/* states */
#include <ctype.h>
#include <stdio.h>		/* printf, scanf, NULL */
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
#include "ff.h"
#include "fs_port.h"
#include "cmd.h"

static char pwd[_MAX_SS]="/";
//
//  function: cmd_mount
//      mount file system, for RAM disk do init
//  parameters
//      argc:   1
//      argv:   none
//
void cmd_mount(int argc, char* argv[])
{
}

//
//  function: cmd_unmount
//      unmount file system, for RAM disk do remove and free resource
//  parameters
//      argc:   1
//      argv:   none
//
void cmd_unmount(int argc, char* argv[])
{
}

//
//  function: cmd_dir
//      list contain of filesystem
//  parameters
//      argc:   1
//      argv:   none
//
void cmd_dir(int argc, char* argv[])
{
    FATFS *fs;
    unsigned int i, j;
    FRESULT res;
    DIR dir;
    FILINFO finfo;
    char path[64]={0};
    unsigned long total, fc, dc;
    DWORD fre_clust, fre_sect, tot_sect;
    
#if _USE_LFN
    finfo.altname[0] = 0;
#endif
    finfo.fname[0] = 0;
    res = fs_opendir(&dir, pwd);
    if (res == FR_OK) 
    {
        total = 0;
        fc = 0;
        dc = 0;
        for (i = 0; path[i]; i++) 
            ;
        path[i++] = '/';
        for (;;) 
        {
            res = fs_readdir(&dir, &finfo);
            if (res != FR_OK || !finfo.fname[0]) 
                break;
            if (_FS_RPATH && finfo.fname[0] == '.') 
                continue;
            j = 0;
            do
                path[i+j] = finfo.fname[j];
            while (finfo.fname[j++]);
            
            printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s\n",
                       (finfo.fattrib & AM_DIR) ? 'D' : '-',
                       (finfo.fattrib & AM_RDO) ? 'R' : '-',
                       (finfo.fattrib & AM_HID) ? 'H' : '-',
                       (finfo.fattrib & AM_SYS) ? 'S' : '-',
                       (finfo.fattrib & AM_ARC) ? 'A' : '-',
                       (finfo.fdate >> 9) + 1980,
                       (finfo.fdate >> 5) & 15,
                       finfo.fdate & 31,
                       (finfo.ftime >> 11),
                       (finfo.ftime >> 5) & 63,
                       finfo.fsize,
                       finfo.fname);
            if  (finfo.fattrib & AM_DIR)
                dc++;
            else
            {
                fc++;
                total += finfo.fsize;
            }
        }
        path[--i] = '\0';
        fs_closedir(&dir);
        
        printf("\n%4lu File(s),%10lu bytes total%4lu Dir(s)\n",fc, total, dc);
    }
    /* Get volume information and free clusters of drive 1 */
    res = fs_getfree("", &fre_clust, &fs);
    if (res)
    {
        printf("Get free space fail\n");
        return res;
    }
    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    /* Print the free space (assuming 512 bytes/sector) */
    printf("%10lu KiB total drive space. %10lu KiB available.\n",
           tot_sect / 2, fre_sect / 2);
    return res;
}


//
//  function: cmd_touch
//      creat a new file
//  parameters
//      argc:   > 3
//      argv:   <filename> [<data>](option)
//
void cmd_touch(int argc, char* argv[])
{
    int i;
    unsigned int bw;    // bytes writed
    FRESULT res;
    FIL file;
    char path[64]={0};
    
    if (argc>1)
    {
        //strcpy(path,pwd);
        //strcat(path,"/");
        strcat(path,argv[1]);
        res = fs_open(&file,path,FA_CREATE_NEW| FA_WRITE);
        if (res == FR_OK)
        {
            for (i=2;i<argc;i++)
            {
                res = fs_write(&file,argv[i],strlen(argv[i]),&bw);
                if ( res!=FR_OK || bw < strlen(argv[i]) )
                {
                    printf("write error %d\n",res);
                    break;
                }
            } 
        }
        else if(res == FR_EXIST)
            printf("create: cannot create file '%s' , File exists. \r\n",path);
        else
            printf("create:cannot create '%s, res=%d'. \r\n",path, res);
        fs_close(&file);
    }
}


//
//  function: cmd_del
//      delete specified file
//  parameters
//      argc:   2
//      argv:   file name
//
void cmd_del(int argc, char* argv[])
{
    FRESULT res;
    char path[64]={0};
    
    if (argc>1)
    {
        //strcpy(path,pwd);
        //strcat(path,"/");
        strcat(path,argv[1]);
        res = fs_unlink(path);
        if (res == FR_OK)
            printf("delete %s OK\n",path);
        else
            printf("cannot delete %s %d\n",path, res);
    }
}