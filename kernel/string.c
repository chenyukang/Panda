
// @Name   : string.c 
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-08 21:51:43
//
// @Brief  :


#include <string.h>

const char* digits = "0123456789";

typedef char* va_list;

extern void putch(char);

#define INTSIZEOF(n) ((sizeof(n)+sizeof(int)-1) & ~(sizeof(int)-1))
#define va_start(ap, format) ( ap = (va_list)(&format) + INTSIZEOF(format))
#define va_end(ap) ( ap=(va_list)0 )
#define va_arg(ap, type) ( *(type*) ((ap += INTSIZEOF(type)) - INTSIZEOF(type)))

void strcpy(char* dest, char* src) {
    char* p = src;
    while(*p) {
        *dest++ = *p++;
    }
}


void strncpy(char* dest, char* src, size_t cnt) {
    while(cnt && (*dest++ = *src++))
        cnt--;
    if(cnt > 0)
        *dest++ = '\0';
}

/* ignore overlap */
void* memcpy(void *dest, const void *src, size_t cnt) {
    const unsigned char *sp = (const unsigned char *)src;
    unsigned char *dp = (unsigned char *)dest;
    while(cnt--){
        *dp++ = *sp++;
    }
    return dest;
}

s32 memcmp(const void* v1, const void* v2, u32 n) {
    const unsigned char *s1, *s2;
    s1 = v1, s2 = v2;
    while( n-- > 0 ) {
        if( *s1 != *s2) {
            return *s1 - *s2;
        }
        s1++, s2++;
    }
    return 0;
}

void* memset(void* addr, unsigned char v, size_t cnt) {
    char* t = (char*)addr;
    while(cnt--){
        *t++ = v;
    }
    return addr;
}

void* memmove(void* dest, const void* src, size_t cnt) {
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

unsigned short *memsetw(unsigned short *dest,
                        unsigned short val, size_t count) {
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

s32 strncmp(const char* v1, const char* v2, u32 n) {
    const char* p = v1;
    const char* q = v2;
    while(n>0 && *p && *p == *q) {
        n--, p++, q++;
    }
    if( n == 0)
        return 0;
    return (u8)*p - (u8)*q;
}

size_t strlen(const char *str) {
    size_t retval;
    for(retval = 0; *str != '\0'; str++)
        retval++;
    return retval;
}


/* Uses the above routine to output a string... */
inline void puts(const char* text) {
    u32 i;
    for (i = 0; i < strlen(text); i++)
    {
        putch(text[i]);
    }
}

inline void printk_hex(u32 val) {
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

inline void printk_int(u32 val) {
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
}

//be careful with 1<<32
static inline char*
int_to_str(char* str, const s32 num,
                        const s32 radix) {
    char* ptr = str;
    char* end ;
    int i, j;
    s32 val = num;

    if(num < 0)
        *ptr++ = '-';
    if(num == 0) {
        *ptr++ = '0';
        end = ptr;
    } else {
        while (val){
            *ptr++  = digits[abs(val % radix)];
            val /= radix;
        }
        end = ptr;
        if(*str == '-')//negative
            str++;

        //now reverse
        j = ptr - str - 1;
        for (i = 0; i < (ptr - str) / 2; i++){
            char temp = str[i];
            str[i]  = str[j];
            str[j--] = temp;
        }
    }
    return end;
}

static inline char*
u32_to_str(char* str, const u32 val) {
    char* ptr = str;
    *ptr++ = '0';
    *ptr++ = 'x';
    int k;
    u32 t;
    for(k=28; k>0; k-=4) {
        t = (val>>k) & 0xF;
        if( t >= 0xA )
            *ptr++ = (t-0xA+'A');
        else
            *ptr++ = (t+'0');
    }
    t = val & 0xF;
    if( t >= 0xA ) *ptr++ = (t-0xA+'A');
    else           *ptr++ = (t+'0');
    return ptr;
}

/* print according format */
int sprintk(char* buf, const char* format, va_list args) {
    int len = 0 ;
    char* prev = buf;
    while(*format){
        if(*format != '%') {
            *buf++ = *format++;
            continue;
        }
        //meet %
        format++;
        switch(*format){
        case 'c':
            *buf++ = va_arg(args, char);
            break;
        case 'd':{
            s32 val = va_arg(args, s32);
            buf = int_to_str(buf, val, 10);
            break;
        }
        case 'f':
            break;
        case 's':{
            char* str = va_arg(args, char*);
            while(*str){
                *(buf++) = *(str++);
            }
            break;
        }
        case 'x':{
            u32 val = va_arg(args, u32);
            buf = u32_to_str(buf, val);
            break;
        }
        }
        format++;
    }
    *buf = '\0';
    return len = strlen(prev);
}

/* NOTE: there is no error check */
int printk(const char* format, ... ) {
    va_list ap;
    char buf[1024];
    u32  cnt = 0, k = 0;
    va_start(ap, format);
    cnt = sprintk(buf, format, ap);
    for(k=0; k<cnt; k++)
        putch(buf[k]);
    va_end(ap);
    return cnt;
}

