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

int list(const char *pathname,const struct stat *statbuf);
int open(DIR *dirp, struct dirent *dread, char *ipathname, long int uid);
int opentime( DIR *dirp, struct dirent *dread, char *ipathname, const int time);
int opennoflag( DIR *dirp, struct dirent *dread, char *ipathname);

int main( int argc, char *argv[] ){
    int i;
    i = getopt(argc, argv, "u:m:"); //shit //this is to get the flag/mode(optopt). optarg is used to get the additional argument (starting path)//
    if( i == -1){
        DIR *dopen = opendir(argv[argc -1]); //opens the starting path directory //
        if( dopen == NULL){
            fprintf(stderr, "Bad starting path %s \n", argv[argc -1]);
            return -1;
        }

        struct dirent *dreads = readdir(dopen); //not obvious why do this
        if(dreads == NULL){
            perror("something wrong reading the directory");
                closedir(dopen);
                return -1;
        }

        char *cpathname = argv[argc -1]; //misleading declare when needed

        while( strcmp(dreads->d_name, ".") == 0 || strcmp(dreads->d_name, "..") == 0 || strcmp(dreads->d_name, ".DS_Store") == 0){
            dreads = readdir(dopen); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
        }

        int i = 1; //shadows code bad practice. shadow declaration of i in line 22
        while (i){ 
            i = opennoflag(dopen, dreads, cpathname); //most of the reading and printing is all done in this funtion and i just call this function recursively. I made a separate function that I call within this open function.//
            if(i == 0){ 
                break;
            }
            dreads = readdir(dopen);
        }
        return 0;
    }
    if(optopt == 'u') {
        int name;
        name = optarg[0] + '\0'; //can compare characters directly
        if( 'A' <= name && name <= 'z' ){ //there's weird characters in between bug  //there is a fucntion in string to check letter //check if the inputted user is by name. Converted the inputted char into int.(I got the conversion from stackoverflow)//
            //open directories with username      
            struct passwd *userinfo = getpwnam(optarg);
            if(userinfo == NULL) {
                perror("Could not get userinfo");
            }

            const char *startingpath = argv[argc - 1];

            DIR *dopen = opendir(star); //opens the starting path directory.... last argument command should be the starting path
            if( dopen == NULL){
                fprintf(stderr, "Bad starting path %s \n", argv[argc - 1]);
                return -1;
            }

            struct dirent *dreads = readdir(dopen);
            if(dreads == NULL){
                perror("something wrong reading the directory");
                    closedir(dopen);
                    return -1;
            }

            char *cpathname = argv[argc -1];

            while( strcmp(dreads->d_name, ".") == 0 || strcmp(dreads->d_name, "..") == 0 || strcmp(dreads->d_name, ".DS_Store") == 0){
                dreads = readdir(dopen); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
            }
            
            int i = 1; //shit
            while (i){ //shit
                i = open(dopen, dreads, cpathname, userinfo->pw_uid); //most of the reading and printing is all done in this funtion and i just call this function recursively. I made a separate function that I call within this open function.//
                if(i == 0){
                    break;
                }
                dreads = readdir(dopen); //need
            }
        } else if ( '0' <= name && name <= '9'){ //is number function exists
            //open directories with uid.
            uid_t uid;
            sscanf(optarg, "%u", &uid);

            DIR *dopen = opendir(argv[argc -1]); //substitute argv[argc - 1] as starting path and initialize it in the beginning because you call it multiple time //opens the starting path directory //
            if( dopen == NULL){
                fprintf(stderr, "Bad starting path %s \n", argv[argc -1]);
                return -1;
            }

            struct dirent *dreads = readdir(dopen);
            if(dreads == NULL){
                perror("something wrong reading the directory");
                    closedir(dopen);
                    return -1;
            }

            char *cpathname = argv[argc -1];

            while( strcmp(dreads->d_name, ".") == 0 || strcmp(dreads->d_name, "..") == 0 || strcmp(dreads->d_name, ".DS_Store") == 0){
                dreads = readdir(dopen); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
            }
            int i = 1; // i is declaraded again. i shadows the i in line 22
            while (i){
                i = open(dopen, dreads, cpathname, uid);
                /*most of the reading and printing is all done in this funtion and i just call this function recursively. 
                I made a separate function that I call within this open function.*/
                if(i == 0){
                    break;
                }
                dreads = readdir(dopen);
            }
        }
        return 0;
    } else if (optopt == 'm') {
        int mstime = atoi(optarg); //modified selected time
        DIR *dopen = opendir(argv[argc -1]); //opens the starting path directory //
        if( dopen == NULL){
            fprintf(stderr, "Bad starting path %s \n", argv[argc -1]);
            return -1;
            }
        struct dirent *dreads = readdir(dopen);
        if(dreads == NULL){
            perror("something wrong reading the directory");
                closedir(dopen);
                return -1;
        }
        char *cpathname = argv[argc - 1];
        while( strcmp(dreads->d_name, ".") == 0 || strcmp(dreads->d_name, "..") == 0 || strcmp(dreads->d_name, ".DS_Store") == 0){
            dreads = readdir(dopen); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
        }
        int i = 1;
        while (i){
            i = opentime(dopen, dreads, cpathname, mstime); //most of the reading and printing is all done in this funtion and i just call this function recursively. I made a separate function that I call within this open function.//
            if(i == 0){
                break;
            }
            dreads = readdir(dopen);
        }
        return 0;
    }
    fprintf(stderr, "invalid flag. can be either -u or -m");
    return -1;
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
    
    perm = statbuf->st_mode; 
    fileType = (perm >> 12);  // shift the bit so the number only represent the type of the node//
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
    //bitwise operations
    if( perm >= (1 << 15)) { perm -= (1 << 15);} //all this is to set the permission mask. This is best way that I can think of integrating.
    if ( perm >= 1 << 14) { perm -= (1 << 14); } 
    if ( perm >= (1 << 13)) { perm -= (1 << 13); } 
    if ( perm >= (1 << 12)) { perm -= (1 << 12); } //horrible indentation wrong type of formatting
    if ( perm >= (1 << 11)) { 
        perm -= (1 << 11);
        uid = 1;
    } if( perm >= (1 << 10)){ perm -= (1 << 10);
        gid = 1;
    } if( perm >= (1 << 9)){ perm -= (1 << 9);
        sticky = 1;
    } if ( perm >= (1<<8)){ perm -= (1 << 8);
        permission[0] = 'r';
    } else {
        permission[0] = '-';
    } if (perm >= (1<<7)){ perm -= (1<<7);
        permission[1] = 'w';
    } else {
        permission[1] = '-';
    } if (perm >= (1<<6)){ perm -= (1<<6);
        if(uid == 1){
            permission[2] = 's'; 
        } else {
            permission[2] = 'x';
        }
    } else {
        if(uid == 1){
            permission [2] = 'S';
        } else {
            permission[2] = '-';
        }
    } 
    
    if (perm >= (1<<5)){ perm -= (1<<5);
        permission[3] = 'r';
    } else {
        permission[3] = '-';
    } 
    
    if (perm >= (1<<4)){ perm -= (1<<4);
        permission[4] = 'w';
    } else {
        permission[4] = '-';
    } 
    
    if (perm >= (1<<3)){ perm -= (1<<3);
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
    
    if( perm >= (1<<2)){ perm -= (1<<2);
        permission[6] = 'r';
    } else {
        permission[6] = '-';
    } 
    
    if( perm >= (1<<1)){ perm -= (1<<1);
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

int open( DIR *dirp, struct dirent *dread, char *ipathname, long int uid){
    struct stat dstatbuf; //this is where i store my info about all the files in the filesystem
    char *npathname = malloc(strlen(ipathname)*sizeof(char)); //maximum allowed file name.
    if( dread == NULL){  //this means there is no more directories //
        closedir(dirp); //this should be where the function will be last if there is not error reading any of the files.
        free(npathname);
        return 0;
    }
    npathname = strcpy(npathname, ipathname); //making a duplicate of the starting pathname. I also found about about this fucntion by asking others//
    if(dread->d_type == DT_DIR){ //check if the file is a directory
        npathname = strcat(strcat(npathname, "/"), dread->d_name);  //creating a new pathname to walk down to with the duplicated pathname. googled this function.
        int dstat = stat(npathname, &dstatbuf);
        if( dstat == -1) {
            perror( "Bad read info");
            free(npathname);
            return 0;
        }
        if ( uid == dstatbuf.st_uid){
            int plist = list(npathname, &dstatbuf); //this is where I print it out.
            if(plist == -1){
                fprintf(stderr,"problem printing %s", npathname);
                free(npathname);
                return 0;
            }
        }
        DIR *dopenr = opendir(npathname);
        if( dopenr == NULL){
            fprintf(stderr, "something wrong opening this directory %s", dread->d_name);
            free(npathname);
            return 0;
        }
        struct dirent *dreadr = readdir(dopenr);
            if(dreadr == NULL){
                closedir(dopenr);
                free(npathname);
                return 0;
            }
        while( strcmp(dreadr->d_name, ".") == 0 || strcmp(dreadr->d_name, "..") == 0 || strcmp(dreadr->d_name, ".DS_Store") == 0){
            dreadr = readdir(dopenr); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
            if(dreadr == NULL){
                closedir(dopenr);
                free(npathname);
                return 0;
            }
        }
        while(1){
            int i = 1;
            i = open(dopenr, dreadr, npathname, uid); //recursively calling the function again
            if (i == 0){
                break;
            }
            dreadr = readdir(dopenr);
        }
    } else { //if the file is anything other than a directory
        char *pathname = strcat(strcat(ipathname, "/"), dread->d_name); 
        int dstat = stat(ipathname, &dstatbuf);
        if( dstat == -1) {
            perror( "Bad read info");
            return 0;
        }
        if ( uid == dstatbuf.st_uid){
            int plist = list(pathname, &dstatbuf);
            if(plist == -1){
                fprintf(stderr,"problem printing %s", pathname);
                return 0;
            }
        }
        pathname[(strlen(pathname)-strlen(dread->d_name)-1)] = '\0';  // this is to get rid of the file name that i was reading. it should work since it automatically add a null terminator in the end because the file names all have a null terminator in the end.
    }
    return 1;
}

int opentime( DIR *dirp, struct dirent *dread, char *ipathname, const int time){
    struct stat dstatbuf; //this is where i store my info about all the files in the filesystem
    char *npathname = malloc(strlen(ipathname)*sizeof(char) + NAME_MAX); //maximum allowed file name.
    if( dread == NULL){  //this means there is no more directories //
        closedir(dirp); //this should be where the function will be last if there is not error reading any of the files.
        free(npathname);
        return 0;
    }
    npathname = strcpy(npathname, ipathname); //making a duplicate of the starting pathname. I also found about about this fucntion by asking others//
    if(dread->d_type == DT_DIR){ //check if the file is a directory
        npathname = strcat(strcat(npathname, "/"), dread->d_name);  //creating a new pathname to walk down to with the duplicated pathname. googled this function.
        int dstat = stat(npathname, &dstatbuf);
        if( dstat == -1) {
            perror( "Bad read info");
            free(npathname);
            return 0;
        }
        if (time >= 0){
            if( time < dstatbuf.st_mtime){
                int plist = list(npathname, &dstatbuf); //this is where I print it out.
                if(plist == -1){
                    fprintf(stderr,"problem printing %s", npathname);
                    free(npathname);
                    return 0;
                }
            }            
        } else if( time > dstatbuf.st_mtime){
            int plist = list(npathname, &dstatbuf); //this is where I print it out.
            if(plist == -1){
                fprintf(stderr,"problem printing %s", npathname);
                free(npathname);
                return 0;
            }
        }
        DIR *dopenr = opendir(npathname);
        if( dopenr == NULL){
            fprintf(stderr, "something wrong opening this directory %s", dread->d_name);
            free(npathname);
            return 0;
        }
        struct dirent *dreadr = readdir(dopenr);
            if(dreadr == NULL){
                closedir(dopenr);
                free(npathname);
                return 0;
            }
        while( strcmp(dreadr->d_name, ".") == 0 || strcmp(dreadr->d_name, "..") == 0 || strcmp(dreadr->d_name, ".DS_Store") == 0){
            printf("dfile is %s \n", dreadr->d_name);
            dreadr = readdir(dopenr); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
            if(dreadr == NULL){
                closedir(dopenr);
                free(npathname);
                return 0;
            }
        }
        while(1){
            int i = 1;
            i = opentime(dopenr, dreadr, npathname, time); //recursively calling the function again
            if (i == 0){
                break;
            }
            dreadr = readdir(dopenr);
        }
    } else { //if the file is anything other than a directory
        char *pathname = strcat(strcat(ipathname, "/"), dread->d_name); 
        int dstat = stat(ipathname, &dstatbuf);
        if( dstat == -1) {
            perror( "Bad read info");
            return 0;
        }
        if (time >= 0){
            if( time < dstatbuf.st_mtime){
                int plist = list(pathname, &dstatbuf); //this is where I print it out.
                if(plist == -1){
                    fprintf(stderr,"problem printing %s", pathname);
                    free(npathname);
                    return 0;
                }
            }            
        } else if( time > dstatbuf.st_mtime){
            int plist = list(pathname, &dstatbuf); //this is where I print it out.
            if(plist == -1){
                fprintf(stderr,"problem printing %s", pathname);
                free(npathname);
                return 0;
            }
        }
        pathname[(strlen(pathname)-strlen(dread->d_name))-1] = '\0';  // this is to get rid of the file name that i was reading. it should work since it automatically add a null terminator in the end because the file names all have a null terminator in the end.
    }
    return 1;
}
int opennoflag( DIR *dirp, struct dirent *dread, char *ipathname){
    while( strcmp(dread->d_name, ".") == 0 || strcmp(dread->d_name, "..") == 0 || strcmp(dread->d_name, ".DS_Store") == 0){
        dread = readdir(dirp); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
        if(dread == NULL){
            closedir(dirp);
            return 1;
        }
    }
    struct stat dstatbuf; //this is where i store my info about all the files in the filesystem
    char *npathname = malloc(strlen(ipathname)*sizeof(char)); //maximum allowed file name.
    if( dread == NULL){  //this means there is no more directories //
        closedir(dirp); //this should be where the function will be last if there is not error reading any of the files.
        free(npathname);
        return 0;
    }
    npathname = strcpy(npathname, ipathname); //making a duplicate of the starting pathname. I also found about about this fucntion by asking others//
    if(dread->d_type == DT_DIR){ //check if the file is a directory
        npathname = strcat(strcat(npathname, "/"), dread->d_name);  //creating a new pathname to walk down to with the duplicated pathname. googled this function.
        int dstat = stat(npathname, &dstatbuf);
        if( dstat == -1) {
            perror( "Bad read info");
            free(npathname);
            return 0;
        }
        int plist = list(npathname, &dstatbuf); //this is where I print it out.
        if(plist == -1){
            fprintf(stderr,"problem printing %s", npathname);
            free(npathname);
            return 0;
        }
        DIR *dopenr = opendir(npathname);
        if( dopenr == NULL){
            fprintf(stderr, "something wrong opening this directory %s", dread->d_name);
            free(npathname);
            return 0;
        }
        struct dirent *dreadr = readdir(dopenr);
            if(dreadr == NULL){
                closedir(dopenr);
                free(npathname);
                return 0;
            }
        while( strcmp(dreadr->d_name, ".") == 0 || strcmp(dreadr->d_name, "..") == 0 || strcmp(dreadr->d_name, ".DS_Store") == 0){
            dreadr = readdir(dopenr); // this is to skip the . and .. and DS_store (Im using a macbook to code this) directories, reading starts in the 'open' function//
            if(dreadr == NULL){
                closedir(dopenr);
                free(npathname);
                return 1;
            }
        }
        while(1){
            int i = 1;
            i = opennoflag(dopenr, dreadr, npathname); //recursively calling the function again
            if (i == 0){
                break;
            }
            dreadr = readdir(dopenr);
        }
    } else { //if the file is anything other than a directory
        char *pathname = strcat(strcat(ipathname, "/"), dread->d_name); 
        int dstat = stat(ipathname, &dstatbuf);
        if( dstat == -1) {
            perror( "Bad read info");
            return 0;
        }
        int plist = list(pathname, &dstatbuf);
        if(plist == -1){
            fprintf(stderr,"problem printing %s", pathname);
            return 0;
        }
        pathname[(strlen(pathname)-strlen(dread->d_name)-1)] = '\0';  // this is to get rid of the file name that i was reading. it should work since it automatically add a null terminator in the end because the file names all have a null terminator in the end.
    }
    return 1;
}