
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

static int *ii = 0;

void handler(int signum)
{
static int cnt=0;
cnt++;
printf("cnt is %d\n",cnt);
int i = 0;
ii = &i;
return;
}
int main(int argc,char ** argv)
{
    signal(SIGSEGV,handler);
    printf("IM BACK\n");
   // *(int *) 0 = 123;
    //printf("IM BACK\n");
    *(int *)ii = 1;
}