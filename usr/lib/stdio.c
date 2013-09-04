#include <stdio.h>
#include <syscall.h>
#include <fcntl.h>

static char buf[BUFSIZE];

FILE _iob[OPEN_MAX] = {
    /* stdin, stdout, stderr */
    { 0, (char *) 0, (char *) 0, _READ, 0 },
    { 0, (char *) 0, (char *) 0, _WRITE, 1 },
    { 0, (char *) 0, (char *) 0, _WRITE | _UNBUF, 2 }
};


int _fillbuf(FILE* fp) {
    int bufsize = 0;
    if( (fp->flag & (_READ | _ERR | _EOF)) != _READ )
        return EOF;
    bufsize = (fp->flag & _UNBUF) ? 1 : BUFSIZE;
    if( fp->base == NULL ) {
        fp->base = (char*)malloc(sizeof(char) * bufsize);
        memset(fp->base, 0, sizeof(char) * bufsize);
    }
    fp->ptr = fp->base;
    fp->cnt = read(fp->fd, fp->ptr, bufsize);
    //printf("got cnt: %d buf: %s\n", fp->cnt, fp->ptr);
    if(--fp->cnt < 0) {
        if(fp->cnt == -1)
            fp->flag |= _EOF;
        else
            fp->flag |= _ERR;
        fp->cnt = 0;
        return EOF;
    }
    return *(fp->ptr++);
}


FILE* fopen(char* filename, const char* mode) {
    FILE* fp = NULL;
    int fd;
    char c = *mode;
    if(c != 'r' && c != 'w' && c != 'a')
        return NULL;
    for(fp = _iob; fp < _iob + OPEN_MAX; fp++) {
        if((fp->flag & (_READ | _WRITE)) == 0)
            break;
    }
    if( fp >= _iob + OPEN_MAX)
        return NULL;
    if( c == 'w') {
        fd = open(filename, O_RDWR, 0);
    } else if( c == 'r') {
        fd = open(filename, O_RDONLY, 0);
    } else {
        /* append, need implement seek */
        fd = open(filename, O_CREATE | O_RDWR, 0);
    }
    if(fd < 0) {
        return NULL;
    }
    fp->fd = fd;
    fp->cnt = 0;
    fp->flag = ( c == 'r') ? _READ : _WRITE;
    fp->base = NULL;
    return fp;
}


int _flushbuf(int c, FILE* fp) {
    char v = (char)c;
    int r, bufsize;

    if(( fp->flag & (_WRITE | _ERR | _EOF)) != _WRITE)
        return EOF;
    if(fp->base == NULL && ((fp->flag & _UNBUF) == 0)) {
        fp->base = (char*)malloc(sizeof(char) * BUFSIZE);
        if(fp->base == NULL)
            fp->flag |= _UNBUF;
        else {
            fp->ptr = fp->base;
            fp->cnt = BUFSIZE - 1;
        }
    }
    if( fp->flag & _UNBUF) {
        if(c == EOF)
            return EOF;
        return write(fp->fd, &v, 1);
    } else {
        if( c != EOF )
            *(fp->ptr++) = v;
        bufsize = fp->ptr - fp->base;
        r = write(fp->fd, fp->base, bufsize);
        fp->ptr = fp->base;
        fp->cnt = BUFSIZE - 1;
        return r == bufsize;
    }
    return 0;
}

int fclose(FILE* fp) {
    if(--(fp->cnt) == 0) {
        fflush(fp);
        fp->flag = 0;
        close(fp->fd);
        if(fp->base) {
            free(fp->base);
            fp->base = NULL;
        }
        fp->ptr = NULL;
        return close(fp->fd);
    }
    return 0;
}

int fprintf(FILE* fp, const char* format, ...) {
    va_list ap;
    char buf[1024];
    u32  cnt = 0, k = 0;
    va_start(ap, format);
    memset(buf, 0, sizeof(buf));
    cnt = _sprintf(buf, format, ap);
    va_end(ap);
    for(k=0; k<cnt; k++)
        putc(buf[k], fp);
    fflush(fp);
    return cnt;
}

int fflush(FILE* fp) {
    int ret = 0;
    int i;
    if(fp == NULL)
        return 0;
    if((fp->flag & _WRITE) == 0)
        return -1;
    _flushbuf(EOF, fp);
    if(fp->flag & _ERR)
        return -1;
    return 0;
}
