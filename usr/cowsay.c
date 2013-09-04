
/*******************************************************************************
 *
 *      cowsay.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-05-07 15:51:24
 *
 *
 *******************************************************************************/

#include <syscall.h>
#include <string.h>

//--------
//< hello >
//--------
const char below[1024] =
    " ^__^ \n"
    " (oo)\\_______ \n"
    "(__)\\        )\\/\\ \n"
    "     ||----w | \n"
    "     ||     || \n"
    "     ^^     ^^ \n";


int main(int argc, char* argv[]) {
    char line[1024];
    int i;
    memset(line, 0, 1024);
    if(argc == 1) {
        strcat(line, "<+ Hello +>");
    } else {
        strcat(line, "<+ ");
        for(i=1; i<argc; i++) {
            strcat(line, argv[i]);
            strcat(line, " ");
        }
        strcat(line, "+>");
    }
    for(i=0; i<strlen(line); i++) {
        printf("%c", '=');
    }
    printf("\n%s\n", line);
    for(i=0; i<strlen(line); i++) {
        printf("%c", '=');
    }
    printf("\n");
    printf("%s", below);
    return 0;
}
