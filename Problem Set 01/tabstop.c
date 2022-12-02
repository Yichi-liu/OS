#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "problem03.h"
#include <string.h>



int main( int argc, char **argv ){
    struct MYSTREAM *fp;
    struct MYSTREAM *fp2;
    if (argc == 4){
	    fp = myfopen("infile.txt",O_RDONLY,4096);   
        if(errno != 0) {
            perror("Error: ");        
            return 255;
        }  
	
        fp2 = myfopen("outfile.txt",O_WRONLY,4096);
        if(errno != 0) {
            perror("Error: ");
            return 255;
        } 
        } else if(argc == 3){
            fp = myfdopen(0,O_RDONLY,4096);
            if(errno != 0) {
                perror("Error: ");
                return 255;
            }
            fp2 = myfopen("outfile.txt",O_WRONLY,4096);
            if(errno != 0) {
                perror("Error: ");
               return 255;
            }
        } else if(argc == 2) {
            fp = myfopen("infile.txt",O_RDONLY,4096);   
            if(errno != 0) {
                perror("Error: ");        
                return 255;
            }
            fp2 = myfdopen(1,O_WRONLY,4096);
            if(errno != 0) {
                perror("Error: ");        
                return 255;
            }
        } else {
            fp = myfdopen(0,O_RDONLY,4096);
            if(errno != 0) {
                perror("Error: ");
                return 255;
            }
            fp2 = myfdopen(1,O_WRONLY,4096);
            if(errno != 0) {
                perror("Error: ");        
                return 255;
            }
        }

    
    
    while(fp2->bufPos < fp2->bufSize && fp->bufPos < fp->bufSize){
    int a = myfgetc(fp);
    if(a != '\t'){
        if(a == -1){
            perror("Error: ");
            return 255;
        }
        if(a == 0){
            myfputc(' ',fp2);
        } else {
            myfputc(a,fp2);
        }
    } else {
        myfputc(' ',fp2);
        myfputc(' ',fp2);
        myfputc(' ',fp2);
        myfputc(' ',fp2);
    }
    }
    if(errno != 0) {
        perror("Error: ");
        return 255;
    }
    
    myfclose(fp);
    if(errno != 0) {
        perror("Error: ");
        return 255;
    }
    
    myfclose(fp2);
    if(errno != 0) {
        perror("Error: ");
        return 255;
    }
    return 0;
}


