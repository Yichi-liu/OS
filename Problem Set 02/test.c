#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <grp.h>
#include <time.h>

int main (){
    int a = 2^15;
    //int a = (1 << 6);
    printf("number is %d \n", a);
    int type;
    int fileType = 49162 >> 12;
    printf("file type is %d", fileType);
        switch(fileType){
        case 0:
            type = 'd'; //set type to deleted/free inode//
            break;
        case 1:
            type = 'n'; //named pipe
            break;
        case 2:
            type = 'c'; //character device
            break;
        case 4:
            type = 'd'; //directory
            break;
        case 6:
            type = 'b'; //block device
            break;
        case 8:
            type = 'r'; //regular file
            break;
        case 10:
            type = 's'; //symlink
            break;
        case 12:
            type = 'n'; //network/IPC socket
            break;
        default:
            type = '-';
            fprintf(stderr, "No such type of file \n");
            return -1;
    }
        printf("type is %c", type);
    return 0;
}

int open(DIR *dirp, char *ipathname, struct stat statbuf, struct passwd *getpwnam){
    struct dirent *dirread = readdir(dirp);
    if( dirread == NULL){  //this means there is no more directories //
        closedir(dirp);
        return -1;
    }
    char *pathname = strcat(strcat(ipathname, "/"), dirread->d_name); 
    int dstat = stat(pathname, &statbuf);
    if( dstat == -1) {
        perror( "Bad read stat");
        return -1;
    }
    if ( getpwnam->pw_uid == statbuf.st_uid){
        int plist = list(pathname, &statbuf);
        if(plist == -1){
            return -1;
        }
    }
    if(dirread->d_type == DT_DIR){
        DIR *dopenr = opendir(dirread->d_name);
        if( dopenr == NULL){
            fprintf(stderr, "something wrong opening this directory %s", dirread->d_name);
            return -1;
        }
        struct dirent *dreadr = readdir(dopenr);
            if(dreadr == NULL){
                closedir(dopenr);
                return -1;
            }
        dreadr = readdir(dopenr); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories 
        dreadr = readdir(dopenr);
        while(1){
            int i;
            i = open(dopenr, pathname, statbuf, getpwnam);
            if (i == -1){
                break;
            }
        }
    }
}