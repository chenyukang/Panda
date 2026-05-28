#ifndef _STDLIB_H_
#define _STDLIB_H_

void* malloc(unsigned int size);
void  free(void* ptr);
void  exit(int ret) __attribute__((noreturn));

#endif//_STDLIB_H_
