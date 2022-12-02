#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


int pp[2];
int child(char c);

main()
{
char buf[4096];
int n;
pipe(pp);
if (!fork())
{
child('A');
return 0;
}
if (!fork())
{
child('B');
return 0;
}
//close(pp[1]); /* THIS LINE */
while ((n=read(pp[0],buf,sizeof buf))>0)
write(1,buf,n);
return 0;
}

int child(char c)
{
char buf[1024];
int i;
memset(buf,c,1024);
for(i=0;i<4096;i++)
if (write(pp[1],buf,1024) != 1024)
perror("omg");
return 0;
}