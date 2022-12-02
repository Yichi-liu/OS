#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>

int retVal;
jmp_buf buf;

void signal_handler(int signum) {
    retVal = -1;
    longjmp(buf, 1);
}

int main(int argc, char **argv) {
    int opt;
    int fd;
    int pf_fd;
    char *src;
    int result;             
    char *pattern;
    char *pattern_file;
    char *curr_file;
    int cVal;
    struct stat st, st2, st3, st4;                      // everything else
    struct stat stdin;                                  // for stdin 
    int pattern_length, file_length, count;
    bool pattern_file_flag = false;
    bool context_flag = false;
    int isStdin;

    while((opt = getopt(argc, argv, "p:c:")) != -1) {
        switch(opt) {
            case 'p': //pattern specification
                pattern = optarg;
                //printf("pattern: %s\n", pattern);
                pattern_file = pattern;
                pattern_file_flag = true;
                pattern_length = strlen(pattern);                 
                break;
            case 'c': //context exists
                cVal = atoi(optarg);
                context_flag = true;
                break;
            default:
                fprintf(stderr, "NOT A VALID FLAG: %s\n", strerror(errno));
                break;
        }
    }
    if (!pattern_file_flag) {
        pattern = argv[optind];
        pattern_length = strlen(pattern);
    }   
    // if there's a -p flag but no input file (argc == optind) or if there's a -c flag but no input file 
    // then a stdin in redirection is necessary
    if ( (optind == argc && pattern_file_flag) || ((optind == (argc - 1))&& !pattern_file_flag)) {   // check for stdin redirection                                           
        if (fstat(0, &stdin) != 0) {                                      // cuz 0 is fd for stdin
            fprintf("Error getting stdin file struct %d: %s", errno, strerror(errno));
        }
        isStdin = 1;                                                    // indicates that there is a stdin redirection 
    }
    if (pattern_file_flag) {                                                    // check whether a pattern file descriptor exists
        int pf_fd = open(pattern_file, O_RDONLY);                               // pattern file, file descriptor                        
        if(pf_fd < 0) {
            fprintf(stderr, "ERROR WITH OPENING PATTERN FILE: %s\n", strerror(errno));
            exit(-1);
        }
        if(fstat(pf_fd, &st3) == -1) {
            fprintf(stderr, "ERROR WITH STAT: %s\n", strerror(errno));
            exit(-1);
        }
        pattern_length = st3.st_size;
        pattern = mmap(NULL, pattern_length, PROT_READ, MAP_PRIVATE, pf_fd, 0);
        if(pattern == (caddr_t) -1) {
            fprintf(stderr, "ERROR WITH MMAP OF PATTERN FILE: %s\n", strerror(errno));
            exit(-1);
        }
    }
    // originally we have agrc as the number of total files 
    // but if we have stdin as our input then there aren't any input files
    // therefore the number of files being looped through is only one "stdin in is the sole input file" 
    int Number_Documents;
    if (isStdin == 1) {   // requires redirection
        if(!pattern_file_flag){
        Number_Documents = 0;                   // so that the number of documents ends at 0 b/c "stdin inb is the sole innput file"
        }
        else {
            Number_Documents = 1;
        }
    }                                           // somehow we also need to make current_file = stdin, right?
    else {
        if(!pattern_file_flag){
            Number_Documents = argc - optind - 1;
        }
        else {
            Number_Documents = argc - optind;
        }            
    }
    //loop through all the files
    for(int i = 0; i < Number_Documents; i++) {
        retVal = 1;

        // if there's a stdin then open has to use use 0 
        if (isStdin == 1) {
            fd = 0;                              // stdin fd is 0
        }
        else {
            if(!pattern_file_flag){
                curr_file = argv[optind + 1];
            }
            else {
                curr_file = argv[optind];
            }               // otherwise open normally and save the specified fd from open sys call
            fd = open(curr_file, O_RDONLY);
            printf("fd: %d\n", fd);
        }

        signal(SIGBUS, signal_handler);                                             // signal handler set up
        if(setjmp(buf) == 1) {
            i++;
            fprintf(stderr, "SIGBUS recieved while processing file %s", curr_file);
            continue;
        }
        if(fd < 0) {
            fprintf(stderr, "Can't open for reading: %s\n", strerror(errno));
            exit(-1);
        }
        if((fstat(fd, &st4)) == -1) {
            fprintf(stderr, "ERROR WITH STAT ON INPUT FILE: %s\n", strerror(errno));
            exit(-1);
        }
        // file size 
        if (isStdin == 1) {
            file_length = 1024;                 // so that it only read 1024 bytes from stdin
        }
        else {
            file_length = st4.st_size;
        }
    
        src = mmap(NULL, file_length, PROT_READ, MAP_PRIVATE, fd, 0);           // memory map for input file
        if(src == (caddr_t) -1) {
            fprintf(stderr, "ERROR WITH MMAP OF INPUT FILE: %s\n", strerror(errno)); // no exit if it can't use mmap on a given input source
            continue;
        }
        char *start = src;
        int remaining = file_length - pattern_length;
        while(remaining >= 0) {//loop through file until the end
            result = memcmp(pattern, src, pattern_length);
            if(result == 0) {
                int diff = file_length-pattern_length-remaining;
                printf("%s:%d ", curr_file, diff);
                if (context_flag) {        
                    char *s_print; //start byte for printing
                    char *e_print; //end byte for printing
                    char *itr;
                    if((src - cVal) > start) {//if the number of specified context bytes does not take you out of bounds of the start, that is where to start print
                        s_print = src - cVal;
                    }
                    else {
                        s_print = start;
                    }
                    if((cVal <= remaining)) {//if the context bytes is less than the amount of remaining bytes in src file, end pring at the current byte + context byte + pattern_length -1 to not double count the start byte. Else, do not add cVal to find the point of the end print location.
                        e_print = src + cVal + pattern_length - 1;
                    }
                    else {
                        e_print = src + pattern_length -1;
                    }
                    for(itr = s_print; itr <= e_print; itr++) {//iterate over the region specified by s_print and e_print
                        char print = (*itr); 
                        printf("%c ", print);
                    }
                    printf("     HEX:");
                    for(itr = s_print; itr <= e_print; itr++) {//iterate over region again to hexdump
                        printf("%hhX ", (*itr));
                    }
                }
                printf("\n");
            }
            src++;
            remaining--;
        }
    }
    return retVal;
}