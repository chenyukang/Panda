#include <string.h>
#include <syscall.h>

int main(int argc, char **argv) {
    printf("now in init main\n");
#if 1
    const char* a = "a";
    const char* b = "a";
    if(strcmp(a, b) == 1) {
        return 0;
    }
#endif
    //int* p = (int*)(0x080006B4);
    //*p = 1;
    //printf("value: %d\n", *p);
    if(fork() > 0) {
        printf("parent\n");
        while(1) {
            ;
        }
        
    } else {
        //printf("child\n");
        while(1) {
            ;
        }
    }
    
#if 0
    int pid = fork();
    printf("pid: %d\n", pid);
    if(pid > 0) {
        //*p = 2;
        //printf("value: %d\n", p);
        printf("parent fuck now\n");
        //printf("value2: %d\n", *p);
    } else {
        while(1) {
            ;
        }
        exec("/sh", NULL);
    }
#endif

#if 0
    while(1) {
        ;
    }
#endif
    return 0;

}
