#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "ff.h"
#include "fs_port.h"
#include "cmd.h"
#include "json-maker.h"
#include "tiny-json.h"

void json_dump( json_t const* json);
void json_parser_example(char *src);
void json_writer_example(char *buf);
int cli_json(int argc, char* argv[]);


//JSON Parser Example
void json_parser_example(char *src)
{
    json_t jtmem[32];
    json_t const *p;

    if(!src)
        return ;

    p = json_create(src,jtmem, sizeof(jtmem)/sizeof(jtmem[0]));
    if ( !p )
    {
        printf("Error parsing JSON file!\n");
        return ;
    }
    json_dump(p);
}

// JSON Writer Example
struct network_info
{
    char ip[16];
    char netmask[16];
    char gateway[16];
};


static int netinfo_to_json(char* dest, struct network_info *src)
{
    char* p = dest;
    if(p)
    {
        p = json_objOpen( p, NULL );
        p = json_str( p, "IP", src->ip );
        p = json_str( p, "NETMASK", src->netmask );
        p = json_str( p, "GATEWAY", src->gateway );
        p = json_objClose( p );
        p = json_end( p );
        return p - dest;
    }
    return 0;
}


void json_writer_example(char *buf)
{
    struct network_info net;
    int ret;
    int f;


    if(!buf)
        return;

    // init variables
    memset(&net,0,sizeof(net));
    strcpy(net.ip,"10.0.0.1");
    strcpy(net.netmask,"255.255.255.0");
    strcpy(net.gateway,"10.0.0.254");

    //output json to buf
    ret = netinfo_to_json(buf, &net);
    if(ret)
    {
        f = fs_open("1:/test.json",FA_CREATE_NEW | FA_WRITE);
        if(f>=0)
        {
            fs_write(f,buf,strlen(buf));
            fs_close(f);
        }
    }
    else
    {
        printf("Error create json file\n");
    }
}



/** Print the value os a json object or array.
  * @param json The handler of the json object or array. */
void json_dump( json_t const* json )
{

    jsonType_t const type = json_getType( json );
    if ( type != JSON_OBJ && type != JSON_ARRAY )
    {
        puts("error");
        return;
    }

    printf( "%s", type == JSON_OBJ? " {": " [" );

    json_t const* child;
    for( child = json_getChild( json ); child != 0; child = json_getSibling( child ) )
    {

        jsonType_t propertyType = json_getType( child );
        char const* name = json_getName( child );
        if ( name ) printf(" \"%s\": ", name );

        if ( propertyType == JSON_OBJ || propertyType == JSON_ARRAY )
            json_dump( child );

        else
        {
            char const* value = json_getValue( child );
            if ( value )
            {
                bool const text = JSON_TEXT == json_getType( child );
                char const* fmt = text? " \"%s\"": " %s";
                printf( fmt, value );
                bool const last = !json_getSibling( child );
                if ( !last ) putchar(',');
            }
        }
    }

    printf( "%s", type == JSON_OBJ? " }": " ]" );

}

void cmd_json(int argc, char* argv[])
{
    char buf[2048];
    int f;

    json_writer_example(buf);  // create a sample file  1:/test.json
//    json_parser_example(buf);

    if(argc <2 )
    {
        printf("Usage: %s <filename>\n",argv[0]);
        return;
    }

    f = fs_open(argv[1],FA_OPEN_EXISTING | FA_READ);
    if(!f)
    {
        printf("Cannot open %s!\n",argv[1]);
        return;
    }

    if(fs_read(f,buf,sizeof(buf)-1) > 0)
        json_parser_example(buf);
    else
    {
        printf("Error! Empty file!\n");
    }
    fs_close(f);
}
