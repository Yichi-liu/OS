#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int z;
void f1();
char buf[16];
main()
{
    int ws= -1;
        f1();
        if (fork()==0){
            f1();
            printf("in fork \n");
        }
        f1();
        
        wait(&ws);
        //printf("%d \n", 65290 >> 8);
        printf("exit status is %d \n",WEXITSTATUS((ws)));
        printf("%d \n", ws);
        perror("something wrong");
        return ws >> 8;
}
void f1()
{
    static int i=10;
    printf("%d\n",i);
    i++;
}
