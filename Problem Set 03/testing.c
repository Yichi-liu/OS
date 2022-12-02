#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>



int main(){
        int exe = execlp("cat","cat","<testfile.out",">testfile2.out",NULL);
        if(exe == -1){
            perror("Something wrong executing the command");
            exit(127);
        }
        return 0;
}