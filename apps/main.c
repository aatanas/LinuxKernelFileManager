#define UTIL_IMPLEMENTATION
#include "utils.h"
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    load_config("scancodes.tbl","ctrl.map");

    char ime_fajla[64];
    char in[4];
    char out[64];
    int fl;
    int len;
    while(1){
        printstr("Unesi fajl: ");
        int len = read(1,ime_fajla,64);
        ime_fajla[len-1]='\0';
        if(!strcmp(ime_fajla,"exit")) _exit(0);
        fl = open(ime_fajla,O_RDONLY);
        while(fgets(in,4,fl)){
            len = process_scancode(atoi(in),out);
            if(len==400) break;
            if(len){
                out[len]='\0';
                printstr(out);
            }
        }
        write(0,"\n",1);
        close(fl);
    }
}
