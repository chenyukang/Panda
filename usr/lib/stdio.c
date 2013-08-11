
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

FILE _iob[OPEN_MAX] = { /* stdin, stdout, stderr */
    { 0, (char *) 0, (char *) 0, _READ, 0 },
    { 0, (char *) 0, (char *) 0, _WRITE, 1 },
    { 0, (char *) 0, (char *) 0, _WRITE | _UNBUF, 2 }
};


int _fillbuf(FILE* stream) {
    return 0;
}

int _flushbuf(int fd, FILE* stream) {
    return 0;
}

FILE* fopen(char* filename, const char* mode) {
    FILE* fp = NULL;
    return fp;
}


int fclose(FILE* stream) {
    return 0;
}


int fprintf(FILE* stream, const char* format, ...) {
    return 0;
}

int fflush(FILE* stream) {
    return 0;
}

int ungetc(int ch, FILE* stream) {
    return 0;
}
