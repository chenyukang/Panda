
/*******************************************************************************
 *
 *      cowsay.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-05-07 15:51:24
 *
 *      COPYRIGHT (C) 2006~2012, Nextop INC., ALL RIGHTS RESERVED.
 *
 *******************************************************************************/


//--------
//< hello >
//--------
const char below[512] =
    "\\ ^__^ \n"
    "\\ (oo)\\_______ \n"
    "(__)\\       )\\/\\ \n"
    "     ||----w | \n"
    "     ||     || \n";


int main(int argc, char* argv) {
    char line[1024];
    memset(line, 0, 1024);
    line[0] = '|';
    int i;
    for(i=0; i<argc-1; i++) {
        strcat(line, argv[i]);
    }
    printf("%s\n", line);
    printf("%s\n", below);
    return 0;
}


