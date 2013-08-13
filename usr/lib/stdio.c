
/*******************************************************************************
 *
 *      stdio.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-08-11 20:22:50
 *
 *      COPYRIGHT (C) 2006~2012, Nextop INC., ALL RIGHTS RESERVED.
 *
 *******************************************************************************/

#include <stdio.h>
#include <syscall.h>
#include <fcntl.h>


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
    }
    fp->ptr = fp->base;
    fp->cnt = read(fp->fd, fp->ptr, bufsize);
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

int _flushbuf(int fd, FILE* stream) {
    return 0;
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


int fclose(FILE* fp) {
    if(--(fp->cnt) == 0) {
        fp->flag = 0;
        close(fp->fd);
        if(fp->base) {
            free(fp->base);
            fp->base = NULL;
        }
        fp->ptr = NULL;
    }
    return 0;
}


int fprintf(FILE* fp, const char* format, ...) {

    return 0;
}

int fflush(FILE* fp) {
    return 0;
}

int ungetc(int ch, FILE* fp) {
    return 0;
}
