#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[]) {
    char bash[] = "/bin/bash\x00";
    char *envp[1] = { NULL };
    char *arg[3] = {"/bin/bash", NULL};
    
    if(geteuid() == 0){
        printf("Already root! :)\n\n");
        exit(0);
    } 
    
    char ch;
    
    printf("my pid is %d, did you give me root?[Y/N]\n", getpid());
    scanf("%c", &ch);
    if(!(ch == 'Y' || ch == 'y')){
        printf("oh, bye!\n");
        exit(0);
    }

    if (geteuid() == 0){
        printf("\e[01;36mSuper powers!\e[00m\n\n");
        execve(bash, arg, envp);
    } else {
        printf("\e[00;31mNo power here! :( \e[00m\n\n");
    }

    return 0;
}

