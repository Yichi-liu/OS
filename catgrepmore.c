#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#define _GNU_SOURCE

void handler (int signum) {
    // if(SIGUSR1 == signum) {
    //     fprintf(stderr, "Number of files read: %d, Number of bytes processed: %d\n", byte_count, file_count);
    // }
    // else {
    //     fprintf(stderr, "*** SIGUSR2 received, moving onto file %d\n", file_count++);
    // }
    //if sigusr1, print total number of files and bytes processed so far which catgrepmore should keep track of, then resume
    //if sigusr2, move onto next input file, pretty much as if EOF was seen. Print a brief informative message to standard error,
    // such as "*** SIGUSR2 received, moving on to file #5". The best way to implement this is with setjmp/longjmp or sigsetjmp/siglongjmp.
}

void grep(char *pattern) {
   printf("in grep function\n");
   printf("pattern in grep: %s\n", pattern);
   execlp("grep", "grep", "-a", pattern, stdin, NULL);
}

void more() {  
    printf("in more function\n");
    execlp("more", "more", NULL);
}

// void partial_write(int byte_count, int write_size,char buf[], FILE *fp){
//     int count = write(buf[byte_count - 1], write_size - byte_count, fp);
//     if(count > 0 && count < 4096 - byte_count) {
//         partial_write(count, write_size - byte_count, buf, fp);
//     }
// }

// void partial_write(int byte_count, int write_size, char buf[], int fd) {
//     int count = write(fd, buf[byte_count - 1], write_size - byte_count);
    
//     if(count > 0 && count < 4096 - byte_count) {
//         partial_write(count, write_size - byte_count, buf, fd);
//     }
// }

int main(int argc, char* argv[]) {
    char buf[4096];
    int pipeone[2] = {0};
    int pipetwo[2] = {0};
    int file_count = 0;
    int byte_count = 0;
    int fd;
    pid_t child_grep, child_more;
    char pattern[PATH_MAX];
    struct sigaction sa;
    int write_size = 4096;
    int read_bytes = 1;
    int write_bytes = 0;
    
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;

    strcpy(pattern, argv[1]);

    //printf("pattern: %s\n", pattern);

    if(sigaction(SIGUSR1, &sa, NULL) == -1) {
        fprintf(stderr, "Error with sigaction for SIGUSR1: %s\n", strerror(errno));
    }
    if(sigaction(SIGUSR2, &sa, NULL) == -1) {
        fprintf(stderr, "Error with sigaction for SIGUSR2: %s\n", strerror(errno));
    }

    printf("Before for loop\n");

    for ( int i = 2; i < argc; i++) { //number of files
        fd = open(argv[i], O_RDONLY);
        printf("before pipe setup\n");
        if (pipe(pipeone) < 0) {
            fprintf(stderr, "error creating pipe one: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        // pipe for more
        if (pipe(pipetwo) < 0) {
            fprintf(stderr, "error creating pipe two: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (((fd = open(argv[i], O_RDONLY)) < 0)) {
            fprintf(stderr, "Error opening infile: %s\n", strerror(errno));
        }
        printf("fd: %d\n", fd);
        file_count++;  
        printf("after file increment\n");
        printf("file_count: %d\n", file_count);
        // printf("fd: %d\n", fd);
        while( read_bytes != 0){
            read_bytes = read(fd, buf, 4096);
            printf("fd: %d\n", fd);
            printf("read byte is %d\n",read_bytes);
            if(read_bytes == -1) { 
                fprintf(stderr, "Error reading from infile: %s/n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            else {
            // instead of byte_count do write_size because we're trying to check the armount of bytes written and compare it to the bytes read
                write_bytes = write(pipeone[1], buf, read_bytes);
                printf("write_bytes is %d\n",write_bytes);
                if (write_bytes == -1) {
                    //check errno is ENINTR if it continue 
                    fprintf(stderr, "Error with write to outpipe\n");
                    exit(EXIT_FAILURE);
                }
                // idk what to do here tbh
                // ... whatever happens once theres a partial write
                else if (write_bytes < read_bytes) {

                    printf("Parital Write occured\n");
                }
                else {
                    byte_count = 1 + write_bytes;
                }
            }
            read_bytes = read(fd, buf, 4096);
        
            // remember to write error messages for dup2, grep and close
            (child_grep = fork());
            
            if (child_grep == 0) {
                //printf("pattern: %s\n", pattern);
                //set stdin of grep process
                dup2(pipeone[0], STDIN_FILENO);
                // child closes up input side of grep child
                close(pipeone[0]);
                //set stdout of grep process to pipe 2
                //dup2(pipetwo[1], STDOUT_FILENO);
                //close(pipetwo[1]);
                // grep to print lines that match
                //execlp("grep", "grep", pattern, NULL); 
                grep(pattern);
                //exit(0);
            }
            //else if (child_grep == -1) {
            //fprintf
            wait(1);
            (child_more = fork());
            if (child_more == 0) {
                printf("in the more\n");
                //set stdin of more process
                dup2(pipetwo[0], STDIN_FILENO);
                close(pipetwo[0]);
                more();
                exit(0);
            }

            wait(1);
            //else if (child_grep == -1) {
            //fprintf
            // else {
            //     printf("hello from parent\n");
            // }
            // add wait
            // wait(child_more)
            // wait(child_grep);
        }
    }
    // partial write at the end b/c it needs to be processed through dup2, get grep'ed and more'ed before it even gets written
    // and a check for partial write function
    return 0;
}

