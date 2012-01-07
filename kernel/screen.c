
// @Name   : screen.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 18:04:10
//
// @Brief  :
/* bkerndev - Bran's Kernel Development Tutorial
*  By:   Brandon F. (friesenb@gmail.com)
*  Desc: Screen output functions for Console I/O
*
*  Notes: No warranty expressed or implied. Use at own risk. */
#include <system.h>

/* These define our textpointer, our background and foreground
*  colors (attributes), and x and y cursor coordinates */
unsigned short *textmemptr;
int attrib = 0x0F;
int csr_x = 10, csr_y = 0;

/* ignore overlap */
void *memcpy(void *dest, const void *src, size_t cnt)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    while(cnt--){
        *dp++ = *sp++;
    }
    return dest;
}


void* memset(void* addr, unsigned char v, size_t cnt)
{
    char* t = (char*)addr;
    while(cnt--){
        *t++ = v;
    }
    return addr;
}

void* memmove(void* dest, const void* src, size_t cnt)
{
    void* ret = dest;
    char* d = (char*)dest;
    const char* s = src;
    if( d<=s || d>=(s+cnt)) {
        while(cnt--){
            *d++ = *s++;
        }
    }
    else { //overlap, copy from high to low
        d += (cnt+1);
        s += (cnt+1);
        while(cnt--){
            *d-- = *s--;
        }
    }
    return ret;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count)
{
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

size_t strlen(const char *str)
{
    size_t retval;
    for(retval = 0; *str != '\0'; str++)
        retval++;
    return retval;
}

unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}


/* Scrolls the screen */
void scroll(void)
{
    unsigned blank, temp;

    /* A blank is defined as a space... we need to give it
    *  backcolor too */
    blank = 0x20 | (attrib << 8);

    /* Row 25 is the end, this means we need to scroll up */
    if(csr_y >= 25)
    {
        /* Move the current text chunk that makes up the screen
        *  back in the buffer by a line */
        temp = csr_y - 25 + 1;
        memcpy (textmemptr, textmemptr + temp * 80, (25 - temp) * 80 * 2);

        /* Finally, we set the chunk of memory that occupies
        *  the last line of text to our 'blank' character */
        memsetw (textmemptr + (25 - temp) * 80, blank, 80);
        csr_y = 25 - 1;
    }
}

/* Updates the hardware cursor: the little blinking line
*  on the screen under the last character pressed! */
void move_csr(void)
{
    unsigned temp;

    /* The equation for finding the index in a linear
    *  chunk of memory can be represented by:
    *  Index = [(y * width) + x] */
    temp = csr_y * 80 + csr_x;

    /* This sends a command to indicies 14 and 15 in the
    *  CRT Control Register of the VGA controller. These
    *  are the high and low bytes of the index that show
    *  where the hardware cursor is to be 'blinking'. To
    *  learn more, you should look up some VGA specific
    *  programming documents. A great start to graphics:
    *  http://www.brackeen.com/home/vga */
    outportb(0x3D4, 14);
    outportb(0x3D5, temp >> 8);
    outportb(0x3D4, 15);
    outportb(0x3D5, temp);
}

/* Clears the screen */
void cls()
{
    unsigned blank;
    int i;

    /* Again, we need the 'short' that will be used to
    *  represent a space with color */
    blank = 0x20 | (attrib << 8);

    /* Sets the entire screen to spaces in our current
    *  color */
    for(i = 0; i < 25; i++)
        memsetw (textmemptr + i * 80, blank, 80);

    /* Update out virtual cursor, and then move the
    *  hardware cursor */
    csr_x = 0;
    csr_y = 0;
    move_csr();
}

/* Puts a single character on the screen */
void putch(unsigned char c)
{
    unsigned short *where;
    unsigned att = attrib << 8;

    /* Handle a backspace, by moving the cursor back one space */
    if(c == 0x08)
    {
        if(csr_x != 0) csr_x--;
    }
    /* Handles a tab by incrementing the cursor's x, but only
    *  to a point that will make it divisible by 8 */
    else if(c == 0x09)
    {
        csr_x = (csr_x + 8) & ~(8 - 1);
    }
    /* Handles a 'Carriage Return', which simply brings the
    *  cursor back to the margin */
    else if(c == '\r')
    {
        csr_x = 0;
    }
    /* We handle our newlines the way DOS and the BIOS do: we
    *  treat it as if a 'CR' was also there, so we bring the
    *  cursor to the margin and we increment the 'y' value */
    else if(c == '\n')
    {
        csr_x = 0;
        csr_y++;
    }
    /* Any character greater than and including a space, is a
    *  printable character. The equation for finding the index
    *  in a linear chunk of memory can be represented by:
    *  Index = [(y * width) + x] */
    else if(c >= ' ')
    {
        where = textmemptr + (csr_y * 80 + csr_x);
        *where = c | att;	/* Character AND attributes: color */
        csr_x++;
    }

    /* If the cursor has reached the edge of the screen's width, we
    *  insert a new line in there */
    if(csr_x >= 80)
    {
        csr_x = 0;
        csr_y++;
    }

    /* Scroll the screen if needed, and finally move the cursor */
    scroll();
    move_csr();
}

/* Uses the above routine to output a string... */
inline void puts(const char* text)
{
    int i;

    for (i = 0; i < strlen(text); i++)
    {
        putch(text[i]);
    }
}

inline void printk(const char* text)
{
    puts(text);
}

inline void printk_hex(u32 val)
{
    int initial = 1;
    int k;
    u32 t;
    puts("0x");
    for(k=28; k>0; k-=4){ //for u32 , k can not be 0
        t = (val>>k) & 0xF;
        if( t==0 && initial )
            continue;
        initial = 0;
        if( t >= 0xA ) putch( t-0xA+'A');
        else         putch( t+'0');
    }
    t = val & 0xF;
    if( t >= 0xA ) putch( t-0xA+'A');
    else          putch( t+'0');
}

inline void printk_int(u32 val)
{
    char buf[12];
    int cnt = 0;
    while(val>0) {
        buf[cnt++] = '0' + val%10;
        val /= 10;
    }
    if(cnt == 0)
        putch('0');
    else {
        while(cnt){
            putch(buf[--cnt]);
        }
    }
    
#if 0
    if(val <= 9 && val >= 0){
        putch('0' + val);
        return;
    }
    int t = 1;
    while(t<val)
        t = t * 10;
    t /= 10;
    while(t) {
        if( val <= 9 && val >= 0){
            putch('0' + val);
            return;
        }else {
            putch( val/t + '0');
            val = val % t;
            t /= 10;
        }
    }
#endif
}

/* Sets the forecolor and backcolor that we will use */
void settextcolor(unsigned char forecolor, unsigned char backcolor)
{
    /* Top 4 bytes are the background, bottom 4 bytes
    *  are the foreground color */
    attrib = (backcolor << 4) | (forecolor & 0x0F);
}

/* Sets our text-mode VGA pointer, then clears the screen for us */
void init_video(void)
{
    textmemptr = (unsigned short *)0xB8000;
    cls();
}
