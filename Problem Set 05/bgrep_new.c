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
    int comp_cnt;
    char *pattern;
    char *pattern_file;
    char *curr_file;
    int cVal;
    struct stat st, st2, st3, st4;                      // everything else
    struct stat stdin;                                  // for stdin 
    int pattern_length, file_length, start;
    bool pattern_file_flag = false;
    bool context_flag = false;

    while((opt = getopt(argc, argv, "c:p:")) != -1) {
        switch(opt) {
            case 'p': //pattern specification
                pattern = optarg;
                if(stat(pattern, &st) == -1) {                              // Stat to check whether pattern if a file or a string
                    fprintf(stderr, "ERROR WITH STAT: %s\n", strerror(errno));
                }
                if((st.st_mode & S_IFMT) == S_IFREG) {                      // Pattern is a reg file
                    pattern_file = pattern;
                    pattern_file_flag = true;
                    start = 3;
                }
                else {                                                      // Pattern is string
                    pattern = argv[optind];
                    pattern_length = strlen(pattern);
                    start = 3;                                              // do we still need this? maybe to distinguish between file and string
                }                                
                break;
            case 'c': //context exists
                cVal = atoi(optarg);
                pattern = argv[optind];
                context_flag = true;
                if(stat(pattern, &st2) == -1) {                              // Stat to check whether pattern if a file or a string
                    fprintf(stderr, "ERROR WITH STAT: %s\n", strerror(errno));
                }
                if((st2.st_mode & S_IFMT) == S_IFREG) {                      // Pattern is a reg file
                    pattern_file = pattern;
                    pattern_file_flag = true;
                    start = 4;
                }      
                else {                                                      // Pattern is string
                    pattern = argv[optind];
                    pattern_length = strlen(pattern);
                    start = 4;                                              // do we still need this? maybe to distinguish between file and string
                }          
                break;
            default:
                pattern = optarg;
                pattern_length = strlen(pattern);
                start = 2;
        }
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
        //char pattern_array[pattern_length];
        pattern = mmap(NULL, pattern_length, PROT_READ, MAP_PRIVATE, pf_fd, 0);
        if(pattern == (caddr_t) -1) {
            fprintf(stderr, "ERROR WITH MMAP OF PATTERN FILE: %s\n", strerror(errno));
            exit(-1);
        }
    }



    //loop through all the files
    for(int i = start; i < argc; i++) {
        retVal = 1;
        curr_file = argv[optind];               
        signal(SIGBUS, signal_handler);                                             // signal handler set up
        if(setjmp(buf) == 1) {
            i++;
            fprintf(stderr, "SIGBUS recieved while processing file %s", curr_file);
            continue;
        }
        fd = open(curr_file, O_RDONLY);
        if(fd < 0) {
            fprintf(stderr, "Can't open for reading: %s\n", strerror(errno));
            exit(-1);
        }
        if((fstat(fd, &st4)) == -1) {
            fprintf(stderr, "ERROR WITH STAT ON INPUT FILE: %s\n", strerror(errno));
            exit(-1);
        }
        file_length = st4.st_size;
        src = mmap(NULL, file_length, PROT_READ, MAP_PRIVATE, fd, 0);           // memory map for input file
        if(src == (caddr_t) -1) {
            fprintf(stderr, "ERROR WITH MMAP OF INPUT FILE: %s\n", strerror(errno));
            exit(-1);
        }
        char *start = src;
        while(file_length > 0) {//loop through file until the end
            int remaining = file_length - pattern_length;
            result = memcmp(pattern, src, 1);
            if(result == 0) {
                printf("%s:%d ", curr_file, file_length-pattern_length-remaining);
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
                        e_print = src+ cVal + pattern_length - 1;
                    }
                    else {
                        e_print = src + pattern_length -1;
                    }
                    for(itr = s_print; itr <= e_print; itr++) {//iterate over the region specified by s_print and e_print
                        char print = (*itr); 
                        if(print > 31 && print < 127) { //need this for some reason, gonna figure it out later
                            print = print;
                        }
                        else {
                            fprintf(stderr, "NOT VALID ARGUMENT: %s\n", strerror(errno));
                        }
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