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

static char pwd[512]={0};
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
    char path[64]="/";
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
        return;
    }
    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    /* Print the free space (assuming 512 bytes/sector) */
    printf("%10lu KiB total drive space. %10lu KiB available.\n",
           tot_sect / 2, fre_sect / 2);
    return;
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

/**
  * @brief  empty directory.
  * @param  path: Object name.
  * @retval FR_OK: if empty OK. Other value if error.
  */
static FRESULT empty_dir (char* path)
{
    UINT i, j;
    FRESULT fr;
    FILINFO fno;
    DIR dir;

    fr = fs_opendir(&dir, path);
    if (fr == FR_OK)
    {
        for (i = 0; path[i]; i++) ;
        path[i++] = '/';
        for (;;)
        {
            fr = fs_readdir(&dir, &fno);
            if (fr != FR_OK || !fno.fname[0]) break;
            if (_FS_RPATH && fno.fname[0] == '.') continue;
            j = 0;
            do
                path[i+j] = fno.fname[j];
            while (fno.fname[j++]);
            if (fno.fattrib & AM_DIR)
            {
                fr = empty_dir(path);
                if (fr != FR_OK) break;
            }
            fr = fs_unlink(path);
            if (fr != FR_OK) break;
        }
        path[--i] = '\0';
    }
    fs_closedir(&dir);

    return fr;
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
    int c, recursive=0;
    char path[64]={0};
    
    while((c=getopt(argc, argv, "r")) != -1)
    {
        switch(c)
        {
            case 'r':
                recursive = 1;
            break;
            default:
            break;
        }
    }        
    
    for ( ; optind < argc; optind++)
    {
        strcat(path,argv[optind]);
        if (recursive)
            res = empty_dir(path);
        else
            res = fs_unlink(path);
        if (res == FR_OK)
            printf("delete %s OK\n",path);
        else
            printf("cannot delete %s %d\n",path, res);
    }
}

//
//  function: cmd_mkdir
//      create a folder
//  parameters
//      argc:   2
//      argv:   file name
//
void cmd_mkdir(int argc, char* argv[])
{
    FRESULT res;
    char path[64]={0};
    
    if (argc>1)
    {
        strcat(path,argv[1]);
        res = fs_mkdir(path);
        if (res == FR_OK)
            printf("create folder %s OK\n",path);
        else
            printf("cannot create %s %d\n",path, res);
    }
}


//
//  function: cmd_cd
//      change current director
//  parameters
//      argc:   1
//      argv:   folder name
//
void cmd_cd(int argc, char* argv[])
{
    FRESULT res;
    char path[64]={0};
    
    if (argc>1)
    {
        strcat(path,argv[1]);
        res = fs_chdir(path);
        if (res == FR_OK)
        {
            res = fs_getcwd(pwd, 511);
            if (res == FR_OK)
            {
                printf("change to %s OK\n",path);
                return;
            }
        }
        printf("change to %s %d\n",path, res);
    }
}

//
//  function: cmd_pwd
//      show current work director
//  parameters
//      argc:   1
//      argv:   folder name
//
void cmd_pwd(int argc, char* argv[])
{
    FRESULT res;
    char path[512]={0};
    
    res = fs_getcwd(path, 511);
    if (res == FR_OK)
        printf("current work folder: %s\n",path);
    else
        printf("show currectly folder fail %d\n",res);
}

//
//  function: cmd_rename
//      change file/folder name
//  parameters
//      argc:   1
//      argv:   folder name
//
void cmd_rename(int argc, char* argv[])
{
    FRESULT res;
    char path[512]={0};
    
    if (argc > 2)
    {
        res = fs_rename(argv[1], argv[2]);
        if (res == FR_OK)
            printf("%s change to %s\n",argv[1], argv[2]);
        else
            printf("change name fail %d\n",res);
    }
}


//
//  function: cmd_stat
//      display file status
//  parameters
//      argc:   1
//      argv:   file name
//
void cmd_stat(int argc, char* argv[])
{
    FILINFO fno;
    FRESULT res;
    
    if (argc > 1)
    {
        res = fs_stat(argv[1], &fno);
        switch (res) 
        {
        case FR_OK:
            printf("Size: %lu\n", fno.fsize);
            printf("Timestamp: %u/%02u/%02u, %02u:%02u\n",
                   (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
                fno.ftime >> 11, fno.ftime >> 5 & 63);
            printf("Attributes: %c%c%c%c%c\n",
               (fno.fattrib & AM_DIR) ? 'D' : '-',
               (fno.fattrib & AM_RDO) ? 'R' : '-',
               (fno.fattrib & AM_HID) ? 'H' : '-',
               (fno.fattrib & AM_SYS) ? 'S' : '-',
               (fno.fattrib & AM_ARC) ? 'A' : '-');
        break;

        case FR_NO_FILE:
            printf("It is not exist.\n");
        break;

        default:
            printf("An error occured. (%d)\n", res);
        }
    }
}



/**
  * @brief  Copy a file.
  * @param  psrc: Current object name.
  * @param  pdst: The destination of object name.
  * @param  fwmode: Mode flags that specifies the types of access
  *		    and open method for the file.
  * @retval FR_OK: if copy OK. Other value if error.
  */
static int copy_file( const char* psrc, const char* pdst, unsigned char fwmode )
{
    FRESULT res;
    UINT br = 0;
    UINT bw = 0;
    BYTE *fbuf = 0;
    FIL	fsrc, fdst;
    unsigned int buffer_size = 2048;

    fbuf = (BYTE *)malloc ( buffer_size );
    if ( fbuf == NULL )
    {
        res = 100;
    }
    else
    {
        if ( fwmode == 0 )
            fwmode = FA_CREATE_NEW;
        else
            fwmode = FA_CREATE_ALWAYS;
        res = fs_open( &fsrc, ( const TCHAR * )psrc, FA_READ | FA_OPEN_ALWAYS );
        if ( res == FR_OK )
        {
            res = fs_open( &fdst, ( const TCHAR * )pdst, FA_WRITE | fwmode );
            if ( res == FR_OK )
            {
                while ( res == FR_OK )
                {
                    res = fs_read( &fsrc, fbuf, buffer_size, ( UINT* )&br );
                    if ( res || br == 0 )
                        break;
                    res = fs_write( &fdst, fbuf, ( UINT )br, ( UINT* )&bw );
                    if ( res || bw < br )
                        break;
                }

                fs_close( &fdst );
            }

            fs_close( &fsrc );
        }
    }
    if (fbuf)
        free( fbuf );

    return res;
}

//
//  function: cmd_cp
//      copy files/folders
//  parameters
//      argc:   2
//      argv:   original file , new file
//
void cmd_cp(int argc, char* argv[])
{
    FRESULT res;
    FILINFO fno;
    int c, overwrite=0;
    char *src=NULL, *dest=NULL;
    
    while((c=getopt(argc, argv, "o")) != -1)
    {
        switch(c)
        {
            case 'o':
                overwrite = 1;
            break;
            default:
            break;
        }
    } 
    
    if (optind < argc)
    { 
        src = argv[optind];
        optind++;
    }
    if (optind < argc)
    { 
        dest = argv[optind];
        optind++;
    }
    
    if (src!=NULL && 
        dest!=NULL  )    
    {
        // if new file/folder exist?
        res = fs_stat(dest, &fno);
        if (res != FR_NO_FILE)
        {
            if (res == FR_OK)
                printf("new file/folder exist\n");
            else
                printf("get file satus error\n");
            goto end_cmd_cp;
        }
        res = fs_stat(src, &fno);
        if (res != FR_OK)
        {
            if (res == FR_NO_FILE)
                printf("file not exits\n");
            else
                printf("get file satus error\n");
            goto end_cmd_cp;
        }
        if (fno.fattrib & AM_DIR)
        {
            // copy folder
            //copy_folder(src,dest,overwrite);
        }
        else
        {
            // copy file
            copy_file(src,dest,overwrite);
        }
        printf("copy %s to %s\n",src, dest);
    }
end_cmd_cp:
    return;    
}

//
//  function: cmd_cat
//      displaying files, combining copies of filse and creating new ones.
//  parameters
//      argc:   2
//      argv:   original file , new file
//
void cmd_cat(int argc, char* argv[])
{
    FRESULT res;
    FILINFO fno;
    FIL dfp,sfp;
    int c, overwrite=0,binary =0;
    unsigned int rlen,wlen,size;
    char *src=NULL, *dest=NULL;
    char *buf=NULL;
    
    
    while((c=getopt(argc, argv, "o")) != -1)
    {
        switch(c)
        {
            case 'o':
                overwrite = 1;
            break;
            case 'b':
                binary = 1;
            break;
            default:
            break;
        }
    } 
    
    if (optind < argc)
    { 
        dest = argv[optind];
        optind++;
    }
    else
    {
        printf("no arguments\n");
        goto end_cmd_cat;
    }    
    
    if (optind < argc)
    { 
        src = argv[optind];
        optind++;
    }

    if (src == NULL)
    {
        // display source file
        buf = malloc(2049);
        if (!buf)
            goto end_cmd_cat;
        memset(buf,0,2049);
        res = fs_open(&dfp, dest, FA_READ);
        if (res == FR_OK)
        {
            res = fs_read(&dfp,buf,2048,&rlen);
            if (res == FR_OK)
            {
                if (binary)
                    dump_frame(buf,rlen,"");
                else
                    printf("%s",buf);
                memset(buf,0,rlen);
            }
            fs_close(&dfp);
        }
        else
            printf("cannot open file '%s': %d\n",dest, res);
        free(buf);
        goto end_cmd_cat;    
    }
    // combing two files
    buf = malloc(2048);
    if (!buf)
        goto end_cmd_cat;
    res = fs_open(&dfp, dest, (FA_OPEN_APPEND|FA_WRITE));
    if (res == FR_OK)
    {
        res = fs_open(&sfp, src, FA_READ);
        if (res == FR_OK)
        {
            if (overwrite)
            {
                // seek to the file head
                res = fs_lseek(&dfp,0);
                if (res != FR_OK)
                    goto end_cat;
            }    
            while(1)
            {    
                res = fs_read(&sfp,buf,2048,&rlen);
                if (res != FR_OK || rlen == 0)
                    break;
                // write to dest file
                res = fs_write(&dfp,buf,rlen,&wlen);
                if (res != FR_OK || wlen < rlen)
                    break;
            } 
        end_cat:    
            fs_close(&sfp);
        }    
        fs_close(&dfp);
    }
    free(buf);
end_cmd_cat:  
    return;  
}    
