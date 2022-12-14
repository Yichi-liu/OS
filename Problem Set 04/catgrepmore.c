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


void grep(char *pattern) {
   //execlp("cat","cat", NULL);
   execlp("grep","grep","-a",pattern, NULL);
}

void more() {  
    //execlp("cat","cat", NULL);
    execlp("more", "more", NULL);
}

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
    int read_bytes = 0;
    int write_bytes = 0;
    int wstatus = 1;
    
    sa.sa_flags = SA_RESTART;

    strcpy(pattern, argv[1]);

    //printf("pattern: %s\n", pattern);

    for(int i = 2; i < argc; i++){
        read_bytes = 1;
        fd = open(argv[i], O_RDONLY);
        if (pipe(pipeone) < 0) {
            fprintf(stderr, "error creating pipe one: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        // pipe for more
        if (pipe(pipetwo) < 0) {
            fprintf(stderr, "error creating pipe two: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (fd < 0) {
            fprintf(stderr, "Error opening infile: %s\n", strerror(errno));
        }
        file_count++;  
        // printf("fd: %d\n", fd);
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
            close(pipeone[1]); //make sure to close everywhere so EOF condition occurs
            // grep to print lines that match
            //execlp("grep", "grep", pattern, NULL); 
            close(pipetwo[1]);
            close(pipetwo[0]);
            grep(pattern);
            //exit(0);
        }

        child_more = fork();
        if(child_more == 0) {
            //set stdin of more process
            close(pipeone[1]);
            close(pipeone[0]);
            dup2(pipetwo[0], STDIN_FILENO);
            close(pipetwo[1]);
            close(pipetwo[0]);
            more();
            //exit(0);
        }

        while(1){
            read_bytes = read(fd, buf, 4096);
            if(read_bytes == -1) { 
                fprintf(stderr, "Error reading from infile: %s/n", strerror(errno));
                exit(EXIT_FAILURE);
            } else if(read_bytes == 0){
                close(fd);
                close(pipeone[1]);
                close(pipeone[0]);
                break;
            }
            else {
            // instead of byte_count do write_size because we're trying to check the armount of bytes written and compare it to the bytes read
                write_bytes = write(pipeone[1], buf, read_bytes);
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
            //read_bytes = read(fd, buf, 4096);
        
            // remember to write error messages for dup2, grep and close
        }
        wait(&wstatus);
        //close(pipetwo[1]);
        //close(pipetwo[0]);
    }
}
