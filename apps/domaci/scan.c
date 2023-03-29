#include <string.h>
#include <fcntl.h>
#define BUFF_SIZE 128

static char prvi_red[BUFF_SIZE];
static char drugi_red[BUFF_SIZE];
static int mn_len;
static char mn_tbl[16][66];
static int shift=0;
static int alt=-1;
static int ctrl=0;
void load_config(const char *scancodes_filename, const char *mnemonic_filename)
{

    int sc = open(scancodes_filename,O_RDONLY);
    int mn = open(mnemonic_filename,O_RDONLY);
    fgets(prvi_red,BUFF_SIZE,sc);
    fgets(drugi_red,BUFF_SIZE,sc);
    char t[3];
    fgets(t,3,mn);
    mn_len = atoi(t);
    int i;
    for(i=0;i<mn_len;i++){
       fgets(mn_tbl[i],66,mn);
    }
    close(sc);
    close(mn);
}

int process_scancode(int scancode, char *buffer)
{
	int result;
    __asm__ __volatile__(
        "cmpl $400, %%eax;"
        "je END;"
        "cld;"

        "cmpl $200,%%eax;"
        "jne 1f;"
        "incl (shift);"
        "xorl %%eax,%%eax;"
        "jmp END;"

        "1: ;"
        "cmpl $300,%%eax;"
        "jne 1f;"
        "decl (shift);"
        "xorl %%eax,%%eax;"
        "jmp END;"

        "1: ;"
        "cmpl $201,%%eax;"
        "jne 1f;"
        "incl (ctrl);"
        "xorl %%eax,%%eax;"
        "jmp END;"

        "1: ;"
        "cmpl $301,%%eax;"
        "jne 1f;"
        "decl (ctrl);"
        "xorl %%eax,%%eax;"
        "jmp END;"

        "1: ;"
        "cmpl $202,%%eax;"
        "jne 1f;"
        "incl (alt);"
        "xorl %%eax,%%eax;"
        "jmp END;"

        "1: ;"
        "cmpl $302,%%eax;"
        "jne 1f;"
        "leal (alt),%%esi;"
        "movsb;"
        "xorl %%eax,%%eax;"
        "movl %%eax,(alt);"
        "decl (alt);"
        "incl %%eax;"
        "jmp END;"

        "1: ;"
        "leal prvi_red,%%esi;"
        "movl (shift),%%ebx;"
        "test %%ebx,%%ebx;"
        "je 1f;"
        "leal drugi_red,%%esi;"

        "1: ;"
        "cmpl $-1,(alt);"
        "je 1f;"
        "addl %%eax,%%esi;"
        "movb (%%esi),%%bl;"
        "subb $0x30,%%bl;"
        "movl $10,%%eax;"
        "imulb (alt);"
        "movb %%al,(alt);"
        "addb %%bl,(alt);"
        "xorl %%eax,%%eax;"
        "jmp END;"

        "1: ;"
        "movl (ctrl),%%ebx;"
        "test %%ebx,%%ebx;"
        "je 1f;"
        "addl %%eax,%%esi;"
        "movb (%%esi),%%bl;"
        "leal mn_tbl,%%esi;"
        "L1: ;"
        "cmpb %%bl,(%%esi);"
        "je L2;"
        "addl $66,%%esi;"
        "loop L1;"
        "xor %%eax,%%eax;"
        "jmp END;"
        "L2: ;"
        "xorl %%eax,%%eax;"
        "movb $10,%%al;"
        "pushl %%edi;"
        "movl %%esi,%%edi;"
        "addl $2,%%edi;"
        "movl $64,%%ecx;"
        "repne scasb;"
        "decl %%edi;"
        "incl %%ecx;"
        "movl $64,%%eax;"
        "subl %%ecx,%%eax;"
        "subl %%eax,%%edi;"
        "movl %%eax,%%ecx;"
        "movl %%edi,%%esi;"
        "popl %%edi;"
        "rep movsb;"
        "jmp END;"

        "1: ;"
        "addl %%eax,%%esi;"
        "xorl %%eax,%%eax;"
        "incl %%eax;"
        "movsb;"

        "END: ;"
        :"=a"(result)
        :"a"(scancode),"D"(buffer),"c"(mn_len)
        :"memory","%ebx","%esi","%edx"
    );
    return result;

}
