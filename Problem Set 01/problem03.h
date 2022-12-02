struct MYSTREAM {
    int fd;
    int mode;
    int bufSize;
    int bufPos;
    char *buf;
}; 

struct MYSTREAM *myfopen(const char *pathname, int mode, int bufsiz);
struct MYSTREAM *myfdopen(int filedesc, int mode, int bufsiz);
int myfgetc(struct MYSTREAM *stream);
int myfputc(int c,struct MYSTREAM *stream);
int myfclose(struct MYSTREAM *stream);
