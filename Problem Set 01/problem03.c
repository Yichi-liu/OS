#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "problem03.h"


struct MYSTREAM *myfopen(const char *pathname, int mode, int bufsiz) {

    int fd;
    struct MYSTREAM *stream = malloc(sizeof(struct MYSTREAM)); //The bufsiz parameter is the size of the character buffer, in bytes. The myfopen function should allocate a struct MYSTREAM using malloc and the return value from the function is a pointer to that, or NULL upon failure. 
    //You must also allocate the buffer, either as part of MYSTREAM by using the C trick of a structure whose last member is an array of indefinite size, or by a second malloc.
    stream->buf = malloc(sizeof(char[bufsiz])); //I tried to allocating the buffer and the struct at the same time, however I keep getting segmentation fault because of that not sure why this way works tho.

    if(stream == NULL){
        errno = ENOMEM;
        return NULL;
    }
    if(bufsiz < 0){
        errno = EINVAL; //If the bufsiz parameter is invalid, you may set errno to EINVAL and return NULL immediately.
        return NULL;
    } 
    if(mode == (O_RDONLY)) {  //myfopen takes a pathname and a mode which is either O_RDONLY or O_WRONLY (the O_RDWR mode is not supported).
        fd = open(pathname, O_RDONLY);
        stream->fd = fd;
        stream->mode = mode;
        stream->bufSize = bufsiz;
        stream->bufPos = 0;
    } else if(mode == (O_WRONLY)) {
        umask(0); //converting permission so it is 0777 convention instead of 0666.
        fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0777);  //In the WRONLY case, the behavior is the same as fopen: open the file for truncation, creation if it doesn’t already exist, and with a permissions mode of 0777.
        stream->fd = fd; //Obviously, myfopen will call the actual system call open, and must save the file descriptor within its struct MYSTREAM.
        stream->mode = mode;
        stream->bufSize = bufsiz;
        stream->bufPos = 0;
    } else {
        errno = EINVAL; //If the mode parameter is invalid, you may set errno to EINVAL and return NULL immediately.
        return NULL;
    }
    return stream;
}

struct MYSTREAM *myfdopen(int filedesc, int mode, int bufsiz) {
    struct MYSTREAM *stream = malloc(sizeof(struct MYSTREAM)); //The bufsiz parameter is the size of the character buffer, in bytes. The myfopen function should allocate a struct MYSTREAM using malloc and the return value from the function is a pointer to that, or NULL upon failure. 
    //You must also allocate the buffer, either as part of MYSTREAM by using the C trick of a structure whose last member is an array of indefinite size, or by a second malloc.
    stream->buf = malloc(sizeof(char[bufsiz])); //I tried to allocating the buffer and the struct at the same time, however I keep getting segmentation fault because of that not sure why this way works tho.
  
    if(stream == NULL){
        errno = ENOMEM;
        return NULL;
    }
    if(bufsiz < 0){
        errno = EINVAL; //If the bufsiz parameter is invalid, you may set errno to EINVAL and return NULL immediately.
        return NULL;
    }
    stream->fd = filedesc; 
    stream->mode = mode;
    stream->bufSize = bufsiz;
    stream->bufPos = 0;
    return stream;
}


int myfgetc(struct MYSTREAM *stream){
    int readByte = 2;
    if(stream->bufPos == 0){
        readByte = read(stream->fd, stream->buf, stream->bufSize);
    }
    stream->bufPos++;
    if(readByte == 0){
        errno = 0;
        return -1;
    }
    if(readByte == -1){
        return -1;
    }
    if (stream->bufPos <= stream->bufSize){
        return stream->buf[stream->bufPos-1];
    }
    return 0;
}

int myfputc(int c,struct MYSTREAM *stream){
    int flush;
    stream->bufPos++;
    if( stream->bufPos-1 >= stream->bufSize){
        flush = write(stream->fd, stream->buf,stream->bufSize);
        if (flush == -1 || flush == 0) {
        return -1;
        }
    }
    stream->buf[stream->bufPos-1] = c;
    return c;
}

int myfclose(struct MYSTREAM *stream) {
    
    if(stream->mode == O_WRONLY){
        int flush;
        flush = write(stream->fd, stream->buf,stream->bufSize);
        if (flush == -1 || flush == 0) {
            return -1;
        }
        int closeFd;
        closeFd = close(stream->fd);
        if(closeFd == -1){
            return -1;
        }
        free(stream);
        return 0;
    } else if (stream->mode == O_RDONLY){
        int closeFd;
        closeFd = close(stream->fd);
        if(closeFd == -1){
            return -1;
        }
        free(stream);
        return 0;
    }
    stderr("the stream is either closed or the stream was not opened for neither reading or writing");
    return 0;
}


