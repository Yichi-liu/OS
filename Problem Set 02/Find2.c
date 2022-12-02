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
#include <limits.h>
#include <getopt.h>
#include <ctype.h>

int openowner(DIR *dir, struct dirent *dread, char *pathname, uid_t uid);
int opentime(DIR *dir, struct dirent *dread, char *pathname, int time);
int open(DIR *dir, struct dirent *dread, char *pathname);
int list(const char *pathname,const struct stat *statbuf);

int main(int argc, char*argv[]){
    int opt;
    char *startingpath = argv[argc - 1];
    DIR *sdopen = opendir(startingpath); //starting directory 
    if( sdopen == NULL){
        fprintf(stderr, "Bad starting path %s \n", argv[argc - 1]);
        return -1;
    }

    struct dirent *sdread = readdir(sdopen);
    if(sdread == NULL){
        perror("something wrong reading the directory");
            closedir(sdopen);
            return -1;
    }
    
    while(strcmp(sdread->d_name, ".") == 0 || strcmp(sdread->d_name, "..") == 0 || strcmp(sdread->d_name, ".DS_Store") == 0){
        sdread = readdir(sdopen); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
    }
    
    while((opt = getopt(argc, argv, "u:m:um:")) != -1){
        switch(opt){
            case 'u': //owner specificed files
                if(isdigit(optarg[0]) == 0){
                    struct passwd *userinfo = getpwnam(optarg);
                    if(userinfo == NULL){
                        perror("Could not get userinfo");
                    }
                    uid_t uid = userinfo->pw_uid; 

                    while(sdread != NULL) { //dread will only be null if there is no more item in a directory
                        int i = openowner(sdopen, sdread, startingpath, uid);
                        if(i == 0){ //additional check if there is any errors
                            break;
                        }
                        sdread = readdir(sdopen);
                    }
                    break;                
                } else {
                    uid_t uid;
                    sscanf(optarg, "%u", &uid); //convert char to int

                    while(sdread != NULL) { //dread will only be null if there is no more item in a directory
                        int i = openowner(sdopen, sdread, startingpath, uid);
                        if(i == 0){ //additional check if there is any errors
                            break;
                        }
                        sdread = readdir(sdopen);
                    }
                    break;
                }
            case 'm': if(opt == optopt){//modified time specified files
                int mstime;
                sscanf(optarg, "%d", &mstime);
                while(sdread != NULL) { //dread will only be null if there is no more item in a directory
                    int i = opentime(sdopen, sdread, startingpath, mstime);
                    if(i == 0){ //additional check if there is any errors
                        break;
                    }
                    sdread = readdir(sdopen);
                }
                break;
            }
            default:
                fprintf(stderr, "invalid flag. can be either -u or -m");
                return -1;
        }
    }
    //no specified flags
    while(sdread != NULL) { //dread will only be null if there is no more item in a directory
        int i = open(sdopen, sdread, startingpath);
        if(i == 0){ //additional check if there is any errors
            break;
        }
        sdread = readdir(sdopen);
    } 
    return 0;
}

int openowner(DIR *dir, struct dirent *dread, char *pathname, uid_t uid){
    struct stat dstatbuf; //this is where i store my info about all the files in the filesystem
    
    char *npathname = malloc(strlen(pathname)*sizeof(char)+PATH_MAX); //maximum allowed file name.
    npathname = strcpy(npathname, pathname);

    while( strcmp(dread->d_name, ".") == 0 || strcmp(dread->d_name, "..") == 0 || strcmp(dread->d_name, ".DS_Store") == 0){
        dread = readdir(dir); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
        if(dread == NULL){
            closedir(dir);
            free(npathname);
            return 1;
        }
    }

    if( dread == NULL){  //this means there is no more directories //
        closedir(dir); //this should be where the function will be last if there is not error reading any of the files.
        free(npathname);
        return 0;
    }

    if(dread->d_type == DT_DIR){
        npathname = strcat(strcat(npathname, "/"), dread->d_name);
        int dstat = stat(npathname, &dstatbuf);
        if(dstat == -1){
            perror("Bad read info");
            free(npathname);
            return 0;
        }

        if(uid == dstatbuf.st_uid){
            int plist = list(npathname, &dstatbuf);
            if(plist == -1){
                fprintf(stderr, "problem printing %s", npathname);
                free(npathname);
                return 0;
            }
        }

        DIR *rdopen = opendir(npathname);
        if(rdopen == NULL){
            fprintf(stderr, "something wrong opening this directory %s", dread->d_name);
            free(npathname);
            return 0;
        }

        struct dirent *rdread = readdir(rdopen);
        if(rdread == NULL){
            closedir(rdopen);
            free(npathname);
            return 0;
        }

        while( strcmp(rdread->d_name, ".") == 0 || strcmp(rdread->d_name, "..") == 0 || strcmp(rdread->d_name, ".DS_Store") == 0){
            rdread = readdir(rdopen); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
            if(rdread == NULL){
                closedir(rdopen);
                free(npathname);
                return 1;
            }
        }
        while(rdread != NULL){
            int ir = 1;
            ir = openowner(rdopen, rdread, npathname, uid);
            if(ir == 0){
                break;
            }
            rdread = readdir(rdopen);
        } 
    } else {
        pathname = strcat(strcat(pathname,"/"), dread->d_name);
        int dstat = stat(pathname, &dstatbuf);
        if(dstat == -1){
            perror("Bad read info");
            return 0;
        }
        if(uid == dstatbuf.st_uid){
            int plist = list(pathname, &dstatbuf);
            if(plist == -1){
                fprintf(stderr,"problem printing %s", pathname);
                return 0;
            }
        }
        pathname[(strlen(pathname)-strlen(dread->d_name)-1)] = '\0';
        /* this is to get rid of the file name that i was reading. 
        it should work since it automatically add a null terminator in the end 
        because the file names all have a null terminator in the end. */
    }
    return 1;
}

int opentime(DIR *dir, struct dirent *dread, char *pathname, int time){
    struct stat dstatbuf; //this is where i store my info about all the files in the filesystem
    
    char *npathname = malloc(strlen(pathname)*sizeof(char)); //maximum allowed file name.
    npathname = strcpy(npathname, pathname);

    while( strcmp(dread->d_name, ".") == 0 || strcmp(dread->d_name, "..") == 0 || strcmp(dread->d_name, ".DS_Store") == 0){
        dread = readdir(dir); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
        if(dread == NULL){
            closedir(dir);
            free(npathname);
            return 1;
        }
    }

    if( dread == NULL){  //this means there is no more directories //
        closedir(dir); //this should be where the function will be last if there is not error reading any of the files.
        free(npathname);
        return 0;
    }

    if(dread->d_type == DT_DIR){
        npathname = strcat(strcat(npathname, "/"), dread->d_name);
        
        int dstat = stat(npathname, &dstatbuf);
        if(dstat == -1){
            perror("Bad read info");
            free(npathname);
            return 0;
        }

        if (time >= 0){
            if(time < dstatbuf.st_mtime){
                int plist = list(npathname, &dstatbuf);
                if(plist == -1){
                    fprintf(stderr, "problem printing %s", npathname);
                    free(npathname);
                    return 0;
                }
            }
        } else if( time > dstatbuf.st_mtime){
            int plist = list(npathname, &dstatbuf);
            if(plist == -1){
                fprintf(stderr, "problem printing %s", npathname);
                free(npathname);
                return 0;
            }
        }

        DIR *rdopen = opendir(npathname);
        if(rdopen == NULL){
            fprintf(stderr, "something wrong opening this directory %s", dread->d_name);
            free(npathname);
            return 0;
        }

        struct dirent *rdread = readdir(rdopen);
        if(rdread == NULL){
            closedir(rdopen);
            free(npathname);
            return 0;
        }

        while( strcmp(rdread->d_name, ".") == 0 || strcmp(rdread->d_name, "..") == 0 || strcmp(rdread->d_name, ".DS_Store") == 0){
            rdread = readdir(rdopen); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
            if(rdread == NULL){
                closedir(rdopen);
                free(npathname);
                return 1;
            }
        }
        while(rdread != NULL){
            int ir = 1;
            ir = opentime(rdopen, rdread, npathname, time);
            if(ir == 0){
                break;
            }
            rdread = readdir(rdopen);
        } 
    } else {
        pathname = strcat(strcat(pathname,"/"), dread->d_name);
        int dstat = stat(pathname, &dstatbuf);
        if(dstat == -1){
            perror("Bad read info");
            return 0;
        }

        if (time >= 0){
            if(time < dstatbuf.st_mtime){
                int plist = list(npathname, &dstatbuf);
                if(plist == -1){
                    fprintf(stderr, "problem printing %s", npathname);
                    free(npathname);
                    return 0;
                }
            }
        } else if( time > dstatbuf.st_mtime){
            int plist = list(npathname, &dstatbuf);
            if(plist == -1){
                fprintf(stderr, "problem printing %s", npathname);
                free(npathname);
                return 0;
            }
        }

        pathname[(strlen(pathname)-strlen(dread->d_name)-1)] = '\0';
        /* this is to get rid of the file name that i was reading. 
        it should work since it automatically add a null terminator in the end 
        because the file names all have a null terminator in the end. */
    }
    return 1;   
}

int open(DIR *dir, struct dirent *dread, char *pathname){
    struct stat dstatbuf; //this is where i store my info about all the files in the filesystem
    
    char *npathname = malloc(strlen(pathname)*sizeof(char)); //maximum allowed file name.
    npathname = strcpy(npathname, pathname);

    while( strcmp(dread->d_name, ".") == 0 || strcmp(dread->d_name, "..") == 0 || strcmp(dread->d_name, ".DS_Store") == 0){
        dread = readdir(dir); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
        if(dread == NULL){
            closedir(dir);
            free(npathname);
            return 1;
        }
    }

    if( dread == NULL){  //this means there is no more directories //
        closedir(dir); //this should be where the function will be last if there is not error reading any of the files.
        free(npathname);
        return 0;
    }

    if(dread->d_type == DT_DIR){
        npathname = strcat(strcat(npathname, "/"), dread->d_name);
        
        int dstat = stat(npathname, &dstatbuf);
        if(dstat == -1){
            perror("Bad read info");
            free(npathname);
            return 0;
        }

        int plist = list(npathname, &dstatbuf);
        if(plist == -1){
            fprintf(stderr, "problem printing %s", npathname);
            free(npathname);
            return 0;
        }

        DIR *rdopen = opendir(npathname);
        if(rdopen == NULL){
            fprintf(stderr, "something wrong opening this directory %s", dread->d_name);
            free(npathname);
            return 0;
        }

        struct dirent *rdread = readdir(rdopen);
        if(rdread == NULL){
            closedir(rdopen);
            free(npathname);
            return 0;
        }

        while( strcmp(rdread->d_name, ".") == 0 || strcmp(rdread->d_name, "..") == 0 || strcmp(rdread->d_name, ".DS_Store") == 0){
            rdread = readdir(rdopen); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
            if(rdread == NULL){
                closedir(rdopen);
                free(npathname);
                return 1;
            }
        }
        while(rdread != NULL){
            int ir = 1;
            ir = open(rdopen, rdread, npathname);
            if(ir == 0){
                break;
            }
            rdread = readdir(rdopen);
        } 
    } else {
        pathname = strcat(strcat(pathname,"/"), dread->d_name);
        int dstat = stat(pathname, &dstatbuf);
        if(dstat == -1){
            perror("Bad read info");
            return 0;
        }

        int plist = list(pathname, &dstatbuf);
        if(plist == -1){
            fprintf(stderr,"problem printing %s", pathname);
            return 0;
        }

        pathname[(strlen(pathname)-strlen(dread->d_name)-1)] = '\0';
        /* this is to get rid of the file name that i was reading. 
        it should work since it automatically add a null terminator in the end 
        because the file names all have a null terminator in the end. */
    }
    return 1;   
}

int list(const char *pathname, const struct stat *statbuf) {
    int perm;
    int type;
    int fileType;
    char permission[10];
    int sticky = 0;
    int gid = 0;
    int uid = 0;
    
    struct tm *time = localtime(&statbuf->st_ctime);
    
    struct passwd *owner = getpwuid(statbuf->st_uid);
    if( owner == NULL){
        perror("problem reading the owner of the file");
        return -1;
    }
    
    struct group *gOwner = getgrgid(statbuf->st_gid);
    if(gOwner == NULL ){
        perror("problem reading the group owner of the file");
        return -1;
    }
     
    fileType = (statbuf->st_mode >> 12);  // shift the bit so the number only represent the type of the node//
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
            type = '-'; //regular file
            break;
        case 10:
            type = 's'; //symlink
            break;
        case 12: //these are magic numbers could have it in the header file
            type = 'n'; //network/IPC socket
            break;
        default:
            type = 'N';
            fprintf(stderr, "No such type of file %s\n", pathname);
            return -1;
    }

    perm = statbuf->st_mode;
    if( perm >= (1 << 15)){ 
        perm -= (1 << 15);
    } //all this is to set the permission mask. This is best way that I can think of integrating.
    if ( perm >= 1 << 14){ 
        perm -= (1 << 14); 
    } 
    if ( perm >= (1 << 13)){
        perm -= (1 << 13); 
    } 
    if ( perm >= (1 << 12)){ 
        perm -= (1 << 12); 
    }
    if ( perm >= (1 << 11)){ 
        perm -= (1 << 11);
        uid = 1;
    } if( perm >= (1 << 10)){ 
        perm -= (1 << 10);
        gid = 1;
    } if( perm >= (1 << 9)){ 
        perm -= (1 << 9);
        sticky = 1;
    } if ( perm >= (1<<8)){ 
        perm -= (1 << 8);
        permission[0] = 'r';
    } else {
        permission[0] = '-';
    } if (perm >= (1<<7)){ 
        perm -= (1<<7);
        permission[1] = 'w';
    } else {
        permission[1] = '-';
    } if (perm >= (1<<6)){ 
        perm -= (1<<6);
        if(uid == 1){
            permission[2] = 's'; 
        } else {
            permission[2] = 'x';
        }
    } else if(uid == 1){
            permission [2] = 'S';
    } else {
            permission[2] = '-';
    } 
    
    if (perm >= (1<<5)){ 
        perm -= (1<<5);
        permission[3] = 'r';
    } else {
        permission[3] = '-';
    } 
    
    if (perm >= (1<<4)){ 
        perm -= (1<<4);
        permission[4] = 'w';
    } else {
        permission[4] = '-';
    } 
    
    if (perm >= (1<<3)){ 
        perm -= (1<<3);
        if(gid == 1){
            permission[5] = 's';
        } else {
            permission[5] = 'x';
        }
    } else {
        if(gid == 1){
            permission[5] = 'S';
        } else {
            permission[5] = '-';
        }
    }     
    if( perm >= (1<<2)){ 
        perm -= (1<<2);
        permission[6] = 'r';
    } else {
        permission[6] = '-';
    } 
    
    if( perm >= (1<<1)){ 
        perm -= (1<<1);
        permission[7] = 'w';
    } else {
        permission[7] = '-';
    } 
    
    if ( perm >= 1){
        if(sticky == 1){
            permission[8] = 't';
        } else {
            permission[8] = 'x'; //im repeating myself. can be permissions to character strings can be loopped
        }
    } else {
        permission [8] = '-';
    }
    permission[9] = '\0';

    if (type == 'c' || type == 'b'){
        int major = major(statbuf->st_dev);
        int minor = minor(statbuf->st_dev);
        printf("%llu  %lld %c%s  %d  %s %s  %d, %d %d-%d-%d %d:%d %s \n", statbuf->st_ino, statbuf->st_blocks, type, permission, statbuf->st_nlink, owner->pw_name, gOwner->gr_name, major, minor, time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, pathname); 
        return 0;
    }

    if (type == 's'){
        char *buf;
        int bufsiz = statbuf->st_size +1;
        int readsym = readlink(pathname, buf, bufsiz);
        if( readsym == -1){
            perror("something wrong reading the symlink \n");
            return -1;
        }
        printf("%llu  %lld %c%s  %d  %s %s  %lld %d-%d-%d %d:%d %s -> \n %s", statbuf->st_ino, statbuf->st_blocks, type, permission, statbuf->st_nlink, owner->pw_name, gOwner->gr_name, statbuf->st_size, time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, pathname, buf); 
    } else {
        printf("%llu  %lld %c%s  %d  %s %s  %lld %d-%d-%d %d:%d %s \n", statbuf->st_ino, statbuf->st_blocks, type, permission, statbuf->st_nlink, owner->pw_name, gOwner->gr_name, statbuf->st_size, time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, pathname); 
    }
    return 0;
} //for some reason it prints the wrong local year and month but everything else works fine//