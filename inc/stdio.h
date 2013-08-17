#ifndef STDIO_H
#define STDIO_H


#define NULL 0
#define EOF  (-1)
#define BUFSIZE 1024
#define OPEN_MAX 20

typedef struct _iobuf {
    int   cnt;
    char* ptr;
    char* base;
    int   flag;
    int   fd;
} FILE;

extern FILE _iob[OPEN_MAX];

#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])

enum _flags {
    _READ  = 01,
    _WRITE = 02,
    _UNBUF = 04,
    _EOF   = 010,
    _ERR   = 020
};

#define feof(p) ((p)->flag & _EOF) != 0)
#define ferror(p) ((p)->flag & _ERR) != 0)
#define fileno(p) ((p)->fd)

int _fillbuf(FILE* fp);
int _flushbuf(int, FILE* fp);

#define getc(p) (--(p)->cnt >= 0 \
                 ? (unsigned char) *(p)->ptr++ : _fillbuf(p))

#define putc(x, p) (--(p)->cnt >= 0 \
                   ? *(p)->ptr++ = (x) : _flushbuf((x), p))

#define ungetc(x, p)  ++(p)->cnt; \
                     (*(--(p)->ptr) = x);

#define putchar(x) putc((x), stdout)

FILE* fopen(char* filename, const char* mode);
int fclose(FILE* fp);
int fflush(FILE* fp);

int fprintf(FILE* fp, const char* format, ...);

#endif
