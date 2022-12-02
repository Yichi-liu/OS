#include <errno.h>
#include <fcntl.h>

int main()
{
int fd = open ("/bogus", O_RDONLY);
}