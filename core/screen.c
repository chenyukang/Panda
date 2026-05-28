
// @Name   : screen.c
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-03 18:04:10
//
// @Brief  : Desc: Screen output functions for Console I/O

// detail can found at
// http://atschool.eduweb.co.uk/camdean/pupils/amac/vga.htm

#include <system.h>
#include <string.h>
#include <asm.h>

static u16 *textmemptr;
static s32 attrib = 0x0E, csr_x = 10, csr_y = 0;
static int esc_state;
static int esc_arg;
static int esc_args[2];
static int esc_argc;

static void scroll(void) {
    unsigned blank, temp;
    /* A blank is defined as a space... we need to give it
    *  backcolor too */
    blank = 0x20 | (attrib << 8);

    /* Row 25 is the end, this means we need to scroll up */
    if(csr_y >= 25) {
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
static void move_csr(void) {
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
    outb(0x3D4, 14);
    outb(0x3D5, temp >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, temp);
}

static void enable_cursor(u8 start, u8 end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}

/* Clears the screen */
void cls() {
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

static void clear_eol(void) {
    unsigned blank = 0x20 | (attrib << 8);
    int x;
    for(x = csr_x; x < 80; x++)
        textmemptr[csr_y * 80 + x] = blank;
    move_csr();
}

static void set_cursor(int row, int col) {
    if(row < 1) row = 1;
    if(col < 1) col = 1;
    if(row > 25) row = 25;
    if(col > 80) col = 80;
    csr_y = row - 1;
    csr_x = col - 1;
    move_csr();
}

static int parse_escape(char c) {
    if(esc_state == 0) {
        if(c == 27) {
            esc_state = 1;
            esc_arg = 0;
            esc_argc = 0;
            esc_args[0] = 0;
            esc_args[1] = 0;
            return 1;
        }
        return 0;
    }

    if(esc_state == 1) {
        if(c == '[') {
            esc_state = 2;
            return 1;
        }
        esc_state = 0;
        return 1;
    }

    if(c >= '0' && c <= '9') {
        esc_arg = esc_arg * 10 + c - '0';
        return 1;
    }

    if(c == ';') {
        if(esc_argc < 2)
            esc_args[esc_argc++] = esc_arg;
        esc_arg = 0;
        return 1;
    }

    if(esc_argc < 2)
        esc_args[esc_argc++] = esc_arg;

    if(c == 'J') {
        if(esc_args[0] == 2)
            cls();
    } else if(c == 'K') {
        clear_eol();
    } else if(c == 'H' || c == 'f') {
        int row = esc_argc > 0 && esc_args[0] ? esc_args[0] : 1;
        int col = esc_argc > 1 && esc_args[1] ? esc_args[1] : 1;
        set_cursor(row, col);
    }

    esc_state = 0;
    return 1;
}

static void putch_internal(char c, int update_cursor) {
    unsigned short *where;
    unsigned att = attrib << 8;

    if(parse_escape(c))
        return;

    /* Handle a backspace, by moving the cursor back one space */
    if(c == 0x08) {
        if(csr_x != 0) csr_x--;
        where = textmemptr + (csr_y * 80 + csr_x);
        *where = ' ' | att;	 /* clear last ch */
    }
    /* Handles a tab by incrementing the cursor's x, but only
    *  to a point that will make it divisible by 8 */
    else if(c == 0x09) {
        csr_x = (csr_x + 8) & ~(8 - 1);
    }
    /* Handles a 'Carriage Return', which simply brings the
    *  cursor back to the margin */
    else if(c == '\r') {
        csr_x = 0;
    }
    /* We handle our newlines the way DOS and the BIOS do: we
    *  treat it as if a 'CR' was also there, so we bring the
    *  cursor to the margin and we increment the 'y' value */
    else if(c == '\n') {
        csr_x = 0;
        csr_y++;
    }
    /* Any character greater than and including a space, is a
    *  printable character. The equation for finding the index
    *  in a linear chunk of memory can be represented by:
    *  index = [(y * width) + x] */
    else if(c >= ' ') {
        where = textmemptr + (csr_y * 80 + csr_x);
        *where = c | att;	/* Character AND attributes: color */
        csr_x++;
    }

    /* If the cursor has reached the edge of the screen's width, we
    *  insert a new line in there */
    if(csr_x >= 80){
        csr_x = 0;
        csr_y++;
    }

    scroll();
    if(update_cursor)
        move_csr();
}

/* Puts a single character on the screen */
void putch(char c) {
    putch_internal(c, 1);
}

void screen_write(char* buf, u32 cnt) {
    u32 i;
    for(i = 0; i < cnt; i++)
        putch_internal(buf[i], 0);
    move_csr();
}

/* Sets the forecolor and backcolor that we will use */
void settextcolor(unsigned char forecolor, unsigned char backcolor) {
    /* Top 4 bytes are the background, bottom 4 bytes
    *  are the foreground color */
    attrib = (backcolor << 4) | (forecolor & 0x0F);
}

/* Sets our text-mode VGA pointer, then clears the screen for us */
void init_video(void) {
    textmemptr = (unsigned short *)0xB8000;
    enable_cursor(14, 15);
    cls();
}

/* add some color property */
void puts_color_str(char* str, unsigned color) {
    char* ptr = str;
    attrib = color & 0xFF;
    while(*ptr != '\0'){
        putch(*ptr);
        ptr++;
    }
    attrib = 0x0A;
}
