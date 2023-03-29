/*
 *	console.c
 *
 * This module implements the console io functions
 *	'void con_init(void)'
 *	'void con_write(struct tty_queue * queue)'
 * Hopefully this will be a rather complete VT102 implementation.
 *
 */

/*
 *  NOTE!!! We sometimes disable and enable interrupts for a short while
 * (to put a word in video IO), but this will work even for keyboard
 * interrupts. We know interrupts aren't enabled when getting a keyboard
 * interrupt, as we use trap-gates. Hopefully all is well.
 */

//#define UTIL_IMPLEMENTATION

#include <linux/sched.h>
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/system.h>
#include <string.h>
#include <sys/stat.h>
//#include <utils.h>

#define SCREEN_START 0xb8000
#define SCREEN_END   0xc0000
#define LINES 25
#define COLUMNS 80
#define NPAR 16

#define DISK 0x301
#define ROOT_ON \
	current->root = iget(DISK,1);\
	current->pwd = current->root;

#define ROOT_OFF \
	iput(current->root);\
	current->root = NULL;\
    current->pwd = NULL;

/*syscall-unistd vs syscalltable-sys.h
kernel haddisk current root uid ???

dirent.h vs dir_entry
inode_table ???
strcmp ali samo kad su isti
volatile int

echo echo
domaci1 fd 0 - 1
ECHO termios
novi c fajl
blink cursor*/

extern void keyboard_interrupt(void);

static unsigned long origin=SCREEN_START;
static unsigned long scr_end=SCREEN_START+LINES*COLUMNS*2;
static unsigned long pos;
static unsigned long x,y;
static unsigned long top=0,bottom=LINES;
static unsigned long lines=LINES,columns=COLUMNS;
static unsigned long state=0;
static unsigned long npar,par[NPAR];
static unsigned long ques=0;
static unsigned char attr=0x07;

/*
 * this is what the terminal answers to a ESC-Z or csi0c
 * query (= vt100 response).
 */
#define RESPONSE "\033[?1;2c"

static inline void gotoxy(unsigned int new_x,unsigned int new_y)
{
	if (new_x>=columns || new_y>=lines)
		return;
	x=new_x;
	y=new_y;
	pos=origin+((y*columns+x)<<1);
}

static inline void set_origin(void)
{
	cli();
	outb_p(12,0x3d4);
	outb_p(0xff&((origin-SCREEN_START)>>9),0x3d5);
	outb_p(13,0x3d4);
	outb_p(0xff&((origin-SCREEN_START)>>1),0x3d5);
	sti();
}

static void scrup(void)
{
	if (!top && bottom==lines) {
		origin += columns<<1;
		pos += columns<<1;
		scr_end += columns<<1;
		if (scr_end>SCREEN_END) {

			int d0,d1,d2,d3;
			__asm__ __volatile("cld\n\t"
				"rep\n\t"
				"movsl\n\t"
				"movl %[columns],%1\n\t"
				"rep\n\t"
				"stosw"
				:"=&a" (d0), "=&c" (d1), "=&D" (d2), "=&S" (d3)
				:"0" (0x0720),
				 "1" ((lines-1)*columns>>1),
				 "2" (SCREEN_START),
				 "3" (origin),
				 [columns] "r" (columns)
				:"memory");

			scr_end -= origin-SCREEN_START;
			pos -= origin-SCREEN_START;
			origin = SCREEN_START;
		} else {
			int d0,d1,d2;
			__asm__ __volatile("cld\n\t"
				"rep\n\t"
				"stosl"
				:"=&a" (d0), "=&c" (d1), "=&D" (d2)
				:"0" (0x07200720),
				"1" (columns>>1),
				"2" (scr_end-(columns<<1))
				:"memory");
		}
		set_origin();
	} else {
		int d0,d1,d2,d3;
		__asm__ __volatile__("cld\n\t"
			"rep\n\t"
			"movsl\n\t"
			"movl %[columns],%%ecx\n\t"
			"rep\n\t"
			"stosw"
			:"=&a" (d0), "=&c" (d1), "=&D" (d2), "=&S" (d3)
			:"0" (0x0720),
			"1" ((bottom-top-1)*columns>>1),
			"2" (origin+(columns<<1)*top),
			"3" (origin+(columns<<1)*(top+1)),
			[columns] "r" (columns)
			:"memory");
	}
}

static void scrdown(void)
{
	int d0,d1,d2,d3;
	__asm__ __volatile__("std\n\t"
		"rep\n\t"
		"movsl\n\t"
		"addl $2,%%edi\n\t"	/* %edi has been decremented by 4 */
		"movl %[columns],%%ecx\n\t"
		"rep\n\t"
		"stosw"
		:"=&a" (d0), "=&c" (d1), "=&D" (d2), "=&S" (d3)
		:"0" (0x0720),
		"1" ((bottom-top-1)*columns>>1),
		"2" (origin+(columns<<1)*bottom-4),
		"3" (origin+(columns<<1)*(bottom-1)-4),
		[columns] "r" (columns)
		:"memory");
}

static void lf(void)
{
	if (y+1<bottom) {
		y++;
		pos += columns<<1;
		return;
	}
	scrup();
}

static void ri(void)
{
	if (y>top) {
		y--;
		pos -= columns<<1;
		return;
	}
	scrdown();
}

static void cr(void)
{
	pos -= x<<1;
	x=0;
}

static void del(void)
{
	if (x) {
		pos -= 2;
		x--;
		*(unsigned short *)pos = (attr<<8)+0x20;
	}
}

static void csi_J(int par)
{
	long count;
	long start;

	switch (par) {
		case 0:	/* erase from cursor to end of display */
			count = (scr_end-pos)>>1;
			start = pos;
			break;
		case 1:	/* erase from start to cursor */
			count = (pos-origin)>>1;
			start = origin;
			break;
		case 2: /* erase whole display */
			count = columns*lines;
			start = origin;
			break;
		default:
			return;
	}
	int d0,d1,d2;
	__asm__ __volatile__("cld\n\t"
		"rep\n\t"
		"stosw\n\t"
		:"=&c" (d0), "=&D" (d1), "=&a" (d2)
		:"0" (count),"1" (start),"2" (0x0720)
		:"memory");
}

static void csi_K(int par)
{
	long count;
	long start;

	switch (par) {
		case 0:	/* erase from cursor to end of line */
			if (x>=columns)
				return;
			count = columns-x;
			start = pos;
			break;
		case 1:	/* erase from start of line to cursor */
			start = pos - (x<<1);
			count = (x<columns)?x:columns;
			break;
		case 2: /* erase whole line */
			start = pos - (x<<1);
			count = columns;
			break;
		default:
			return;
	}
	int d0,d1,d2;
	__asm__ __volatile__("cld\n\t"
		"rep\n\t"
		"stosw\n\t"
		:"=&c" (d0), "=&D" (d1), "=&a" (d2)
		:"0" (count),"1" (start),"2" (0x0720)
		:"memory");
}

void csi_m(void)
{
	int i;

	for (i=0;i<=npar;i++)
		switch (par[i]) {
			case 0:attr=0x07;break;
			case 1:attr=0x0f;break;
			case 4:attr=0x0f;break;
			case 7:attr=0x70;break;
			case 27:attr=0x07;break;
		}
}

static inline void set_cursor(void)
{
	cli();
	outb_p(14,0x3d4);
	outb_p(0xff&((pos-SCREEN_START)>>9),0x3d5);
	outb_p(15,0x3d4);
	outb_p(0xff&((pos-SCREEN_START)>>1),0x3d5);
	sti();
}

static void respond(struct tty_struct * tty)
{
	char * p = RESPONSE;

	cli();
	while (*p) {
		PUTCH(*p,tty->read_q);
		p++;
	}
	sti();
	copy_to_cooked(tty);
}

static void insert_char(void)
{
	int i=x;
	unsigned short tmp,old=0x0720;
	unsigned short * p = (unsigned short *) pos;

	while (i++<columns) {
		tmp=*p;
		*p=old;
		old=tmp;
		p++;
	}
}

static void insert_line(void)
{
	int oldtop,oldbottom;

	oldtop=top;
	oldbottom=bottom;
	top=y;
	bottom=lines;
	scrdown();
	top=oldtop;
	bottom=oldbottom;
}

static void delete_char(void)
{
	int i;
	unsigned short * p = (unsigned short *) pos;

	if (x>=columns)
		return;
	i = x;
	while (++i < columns) {
		*p = *(p+1);
		p++;
	}
	*p=0x0720;
}

static void delete_line(void)
{
	int oldtop,oldbottom;

	oldtop=top;
	oldbottom=bottom;
	top=y;
	bottom=lines;
	scrup();
	top=oldtop;
	bottom=oldbottom;
}

static void csi_at(int nr)
{
	if (nr>columns)
		nr=columns;
	else if (!nr)
		nr=1;
	while (nr--)
		insert_char();
}

static void csi_L(int nr)
{
	if (nr>lines)
		nr=lines;
	else if (!nr)
		nr=1;
	while (nr--)
		insert_line();
}

static void csi_P(int nr)
{
	if (nr>columns)
		nr=columns;
	else if (!nr)
		nr=1;
	while (nr--)
		delete_char();
}

static void csi_M(int nr)
{
	if (nr>lines)
		nr=lines;
	else if (!nr)
		nr=1;
	while (nr--)
		delete_line();
}

static int saved_x=0;
static int saved_y=0;

static void save_cur(void)
{
	saved_x=x;
	saved_y=y;
}

static void restore_cur(void)
{
	x=saved_x;
	y=saved_y;
	pos=origin+((y*columns+x)<<1);
}


static int sel;
static int maxsel;
static int len;
int f3_svic;
static int f2mode;
static char* offset;
static char  tabela[10][21];
static char path[16][16]={"/"};
static int dub;
static int prazan;
struct dir_entry* dir;
struct m_inode* ind;
struct buffer_head* bh;

static void crtaj(char c,char boja,char* blok){
    __asm__ __volatile__(
        "cld;"
        "movb %%bl,%%ah;"
        "stosw;"
        ::"a"(c),"b"(boja),"D"(blok)
        );
}

static void crtaj_sred(char* c,int len,char* blok){
    int i;
    blok+=20-len>>1<<1;
    for(i=0;i<len;i++){
        crtaj(c[i],attr,blok+=2);
    }

}

static void putstr(char *s){
    len = strlen(s);
    int i;
    for(i=0;i<len;i++){
        PUTCH(s[i],tty_table[0].read_q);
    }

}

static void select(void){
    int i;
    for(i=1;i<40;i+=2)
        offset[i] = ~offset[i];
}

static void clipboard(void){
    sel = 0;
    len = strlen(tabela[sel]);
    char c[]="[ clipboard ]";
    crtaj_sred(c,strlen(c),offset);
    offset+=0xA0;
    select();
}

static int open_dir(unsigned short ind_num){
    ind=iget(DISK,ind_num);
    if(ind->i_size<=32){
        iput(ind);
        return 0;
    }
    bh = read_file_block(ind,0);
    dir=(struct dir_entry*)bh->b_data;
    dir+=2;
    iput(ind);
    return ind_num;
}

static void explorer(void){
    ROOT_ON
    sel=0;
    char title[21]="[ ";
    strcat(title,path[dub]);
    crtaj_sred(strcat(title," ]"),strlen(title)+2,offset);
    offset+=0xA0;
    if(prazan) goto hehe;
    if(!dub) open_dir(1); //(dir+maksel)->name
    for(maxsel=0;dir[maxsel].inode&&maxsel<10;maxsel++){
        ind = iget(DISK,(dir[maxsel].inode));
        if(S_ISDIR(ind->i_mode)) attr=0x09;
        else if(S_ISCHR(ind->i_mode)) attr=0x0E;
        else if(ind->i_mode & 0111) attr=0x02;
        crtaj_sred(dir[maxsel].name,strlen(dir[maxsel].name),offset);
        offset+=0xA0;
        attr=0x07;
        iput(ind);
    }
    offset=(char*)origin+0x116;
    select();
    hehe: ROOT_OFF
}

void strel2(char s){
    if(f3_svic||f2mode||!s&&!dub||prazan&&s) return;
    ROOT_ON
    struct dir_entry* tmp=dir;
    if(s) tmp+=sel;
    else tmp--;
    ind=iget(DISK,tmp->inode);
    if(S_ISDIR(ind->i_mode)){
        if(open_dir(tmp->inode)){
            prazan=0;
            if(s) goto hihi;
            else dub--;
        }
        else{prazan=1;
            hihi:
            dub++;
            strcpy(path[dub],tmp->name);
        }
        okvir(0);
    }
    else iput(ind);
    ROOT_OFF
}

void unos(void){
    f3_svic ^= 1;
	attr = ~attr;
	if(f3_svic){
		len=strlen(tabela[sel]);
		save_cur();
		gotoxy(59+len,1+sel);
	}
    else restore_cur();
	set_cursor();
}

void strel(char s){
	if(f3_svic||!f2mode&&prazan) return;
    select();
    if(s){
        if(++sel==10||!f2mode&&sel==maxsel){
            sel=0;
            if(f2mode) offset-=9*0xA0;
            else offset-=(maxsel-1)*0xA0;
        }
        else offset+=0xA0;
    }
    else{
        if(--sel==-1){
            if(f2mode) sel = 9;
            else sel = maxsel-1;
            offset+=sel*0xA0;
        } else offset-=0xA0;
    }
    select();
    if(f2mode) len=strlen(tabela[sel]);
}

void okvir(char s){
    if(f3_svic) return;
    f2mode = s;
    offset = (char*)origin+0x74;
    int i,j;
    for(i=0;i<12;i++){
        if(i==0||i==11)
            for(j=0;j<22;j++)
                crtaj('#',attr,offset+(j<<1));
        else{
            crtaj('#',attr,offset);
            crtaj('#',attr,offset+0x2A);
            for(j=1;j<21;j++)
            if(f2mode) crtaj(tabela[i-1][j-1],attr,offset+(j<<1));
            else crtaj(' ',attr,offset+(j<<1));
        }
        offset+=0xA0;
    }
    offset=(char*)origin+0x76;
    if(f2mode) clipboard();
    else explorer();
}

void spejs(){
    if(f3_svic) return;
    int i=0;
    if(f2mode){
        if(len){
            putstr(tabela[sel]);
            do_tty_interrupt(0);
        }
    }
    else {
        int j;
        for(i=0;i<dub+1;i++){
            putstr(path[i]);
            if(i>0) PUTCH('/',tty_table[0].read_q);
        }
        if(!prazan) putstr(dir[sel].name);
        do_tty_interrupt(0);
    }
}

void con_write(struct tty_struct * tty)
{
	int nr;
	char c;
	nr = CHARS(tty->write_q);
	while (nr--) {
		GETCH(tty->write_q,c);
		switch(state) {
			case 0:
				if (c>31 && c<127) {
					if (x>=columns) {
						x -= columns;
						pos -= columns<<1;
						lf();
					}
					if(f3_svic){
					    if(x==79) return;
					    tabela[sel][len++]=c;
				    }
				    __asm__(
                    "movb attr,%%ah;"
                    "movw %%ax,%1;"
                    ::"a" (c),"m" (*(short *)pos)
                    );
                    pos += 2;
                    x++;
				}
				else if (c==ERASE_CHAR(tty)){
					if(f3_svic){
					    if(len){
					        tabela[sel][--len]=0;
                            del();
                        }
					}
					else del();
				}
				else if(f3_svic) break;
				else if (c==27)
					state=1;
				else if (c==10 || c==11 || c==12)
					lf();
				else if (c==13)
					cr();
				else if (c==8) {
					if (x) {
						x--;
						pos -= 2;
					}
				} else if (c==9) {
					c=8-(x&7);
					x += c;
					pos += c<<1;
					if (x>columns) {
						x -= columns;
						pos -= columns<<1;
						lf();
					}
					c=9;
				}
				break;
			case 1:
				state=0;
				if (c=='[')
					state=2;
				else if (c=='E')
					gotoxy(0,y+1);
				else if (c=='M')
					ri();
				else if (c=='D')
					lf();
				else if (c=='Z')
					respond(tty);
				else if (x=='7')
					save_cur();
				else if (x=='8')
					restore_cur();
				break;
			case 2:
				for(npar=0;npar<NPAR;npar++)
					par[npar]=0;
				npar=0;
				state=3;
				if ((ques=(c=='?')))
					break;
			case 3:
				if (c==';' && npar<NPAR-1) {
					npar++;
					break;
				} else if (c>='0' && c<='9') {
					par[npar]=10*par[npar]+c-'0';
					break;
				} else state=4;
			case 4:
				state=0;
				switch(c) {
					case 'G': case '`':
						if (par[0]) par[0]--;
						gotoxy(par[0],y);
						break;
					case 'A':
						if (!par[0]) par[0]++;
						gotoxy(x,y-par[0]);
						break;
					case 'B': case 'e':
						if (!par[0]) par[0]++;
						gotoxy(x,y+par[0]);
						break;
					case 'C': case 'a':
						if (!par[0]) par[0]++;
						gotoxy(x+par[0],y);
						break;
					case 'D':
						if (!par[0]) par[0]++;
						gotoxy(x-par[0],y);
						break;
					case 'E':
						if (!par[0]) par[0]++;
						gotoxy(0,y+par[0]);
						break;
					case 'F':
						if (!par[0]) par[0]++;
						gotoxy(0,y-par[0]);
						break;
					case 'd':
						if (par[0]) par[0]--;
						gotoxy(x,par[0]);
						break;
					case 'H': case 'f':
						if (par[0]) par[0]--;
						if (par[1]) par[1]--;
						gotoxy(par[1],par[0]);
						break;
					case 'J':
						csi_J(par[0]);
						break;
					case 'K':
						csi_K(par[0]);
						break;
					case 'L':
						csi_L(par[0]);
						break;
					case 'M':
						csi_M(par[0]);
						break;
					case 'P':
						csi_P(par[0]);
						break;
					case '@':
						csi_at(par[0]);
						break;
					case 'm':
						csi_m();
						break;
					case 'r':
						if (par[0]) par[0]--;
						if (!par[1]) par[1]=lines;
						if (par[0] < par[1] &&
						    par[1] <= lines) {
							top=par[0];
							bottom=par[1];
						}
						break;
					case 's':
						save_cur();
						break;
					case 'u':
						restore_cur();
						break;
				}
		}
	}
	set_cursor();
}

/*
 *  void con_init(void);
 *
 * This routine initalizes console interrupts, and does nothing
 * else. If you want the screen to clear, call tty_write with
 * the appropriate escape-sequece.
 */
void con_init(void)
{
	register unsigned char a;

	gotoxy(*(unsigned char *)(0x90000+510),*(unsigned char *)(0x90000+511));
	set_trap_gate(0x21,&keyboard_interrupt);
	outb_p(inb_p(0x21)&0xfd,0x21);
	a=inb_p(0x61);
	outb_p(a|0x80,0x61);
	outb(a,0x61);
}
