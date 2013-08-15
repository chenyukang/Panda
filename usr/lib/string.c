
// @Name   : string.c
//
// @Author : Yukang Chen (moorekang@gmail.com)
// @Date   : 2012-01-08 21:51:43
//
// @Brief  :


#include <string.h>
#include <syscall.h>

#define putch write

const char* digits = "0123456789";

int atoi(char* s) {
    int k = 0;
    char* p = s;
    while(!isdigit(*p))
        p++;
    while(isdigit(*p)) {
        k = k * 10 + *p - '0';
        p++;
    }
    return k;
}

int isspace(char c) {
    u32 v = (u32)c;
    if( v == 0x20 || v == 0x09 ||
        v == 0x0a || v == 0x0b ||
        v == 0x0c || v == 0x0d )
        return 1;
    return 0;
}

int isalpha(char c) {
    if (( c >= 'a' && c <= 'z' ) ||
        ( c >= 'A' && c <= 'Z' ))
        return 1;
    return 0;
}

int isdigit(char c) {
    if( c >= '0' && c <= '9')
        return 1;
    return 0;
}

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
    const u8 *sp = (const u8 *)src;
    u8 *dp = (u8 *)dest;
    while(cnt--){
        *dp++ = *sp++;
    }
    return dest;
}

void* strcat(char* dest, const char* src) {
    char* p = dest;
    const char* q = src;
    while(*p != 0)
        p++;
    while(*q != 0)
        *p++ = *q++;
    *p++ = 0;
    return dest;
}

s32 memcmp(const void* v1, const void* v2, u32 n) {
    const u8 *s1, *s2;
    s1 = v1, s2 = v2;
    while( n-- > 0 ) {
        if( *s1 != *s2) {
            return *s1 - *s2;
        }
        s1++, s2++;
    }
    return 0;
}

void* memset(void* addr, u8 v, size_t cnt) {
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

u16* memsetw(u16 *dest, u16 val, size_t count) {
    u16 *temp = (u16 *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

s32 strcmp(const char* v1, const char* v2) {
    const char* p = v1;
    const char* q = v2;
    while(*p && *q && *p == *q) {
        p++, q++;
    }
    if(*p) return 1;
    if(*q) return -1;
    return (u8)*p - (u8)*q;
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
    for (i = 0; i < strlen(text); i++) {
        write(0, (char*)&text[i], 1);
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
int _sprintf(char* buf, const char* format, va_list args) {
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

int printf(const char* format, ... ) {
    va_list ap;
    char buf[1024];
    u32  cnt = 0, k = 0;
    va_start(ap, format);
    cnt = _sprintf(buf, format, ap);
    for(k=0; k<cnt; k++){
        write(1, (char*)(&buf[k]), 1);
    }
    va_end(ap);
    return cnt;
}

int sprintf(char* res, const char* format, ... ) {
    va_list ap;
    char buf[1024];
    u32  cnt = 0, k = 0;
    va_start(ap, format);
    cnt = _sprintf(buf, format, ap);
    for(k=0; k<cnt; k++) {
        res[k] = buf[k];
    }
    va_end(ap);
    res[cnt] = 0;
    return cnt;
}
