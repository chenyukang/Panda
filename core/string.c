
// @Name   : string.c
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-08 21:51:43
//
// @Brief  : string util, this is used both by kernel and user space program,
//           defined USR will be compiled for user space.


#include <string.h>
#include <screen.h>

const static char* digits = "0123456789";

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

unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count) {
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

s32 strcmp(const char* v1, const char* v2) {
    while(*v1 && *v1 == *v2)
        v1++, v2++;
    return (u8)*v1 - (u8)*v2;
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

void* strcat(char* v1, const char* v2) {
    char* p = v1;
    const char* q = v2;
    while(*p != 0)
        p++;
    while(*q != 0) {
        *p++ = *q++;
    }
    return v1;
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
    for (i = 0; i < strlen(text); i++) {
        putch(text[i]);
    }
}

//be careful with 1<<32
static inline char*
int_to_str(char* str, const s32 num, const s32 radix) {
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
static int _printk(char* buf, const char* format, va_list args) {
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

int printk(const char* format, ... ) {
    va_list ap;
    char buf[1024];
    u32  cnt = 0, k = 0;
    va_start(ap, format);
    cnt = _printk(buf, format, ap);
    for(k=0; k<cnt; k++)
        putch(buf[k]);
    va_end(ap);
    return cnt;
}

int sprintk(char* res, const char* format, ... ) {
    va_list ap;
    char buf[1024];
    u32  cnt = 0, k = 0;
    va_start(ap, format);
    cnt = _printk(buf, format, ap);
    for(k=0; k<cnt; k++) {
        res[k] = buf[k];
    }
    va_end(ap);
    res[cnt] = 0;
    return cnt;
}
