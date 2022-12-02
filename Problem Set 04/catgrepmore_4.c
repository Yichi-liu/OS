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

jmp_buf j_buf;
jmp_buf j_buf2;

void handler (int signum) {
    if(signum == SIGUSR1) {
        longjmp(j_buf, 1);
    }
    else {
        longjmp(j_buf2, 1);
    }
}

void grep(char *pattern) {
   execlp("grep","grep","-a",pattern, NULL);
}

void more() {
    execlp("more", "more", NULL);
}

void partial_write(int read_bytes, int write_size, char *buf, int fd) {
    buf += write_size;
    int count = write(fd, buf, write_size - read_bytes);
    if(count > 0 && count < write_size - read_bytes) {
        partial_write(count, write_size - read_bytes, buf, fd);
    } else if (count < 0){
        if(errno == EINTR) {
            return;
        }
        fprintf(stderr, "Error with partial write to outpipe\n");
        exit(EXIT_FAILURE);
    }
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
    int write_size = 4096;
    int read_bytes = 0;
    int write_bytes = 0;
    int wstatus = 1;

    strcpy(pattern, argv[1]);

    for(int i = 2; i < argc; i++) {
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
        (void) signal(SIGUSR1, handler);
        if(setjmp(j_buf) != 0) {
            fprintf(stderr, "Number of files read: %d, Number of bytes processed: %d\n", file_count, byte_count);
        }
        (void) signal(SIGUSR2, handler);
        if(setjmp(j_buf2) != 0) {
            int temp = file_count;
            fprintf(stderr, "*** SIGUSR2 received, moving on to file #%d\n", temp++);
            continue;
        }

        child_grep = fork();
        if (child_grep == 0) {
            //set stdin of grep process
            if (dup2(pipeone[0], STDIN_FILENO) == -1) {
                fprintf(stderr, "Error with dup2 on pipeone[0] in grep child: %s\n", strerror(errno));
            }
            // child closes up input side of grep child
            if (close(pipeone[0]) == -1) {
                fprintf(stderr, "Error with close on pipeone[0] in grep child: %s\n", strerror(errno));
            }
            //set stdout of grep process to pipe 2
            if (close(pipeone[1]) == -1) {
                fprintf(stderr, "Error with close on pipeone[1] in grep child: %s\n", strerror(errno));
            }
            // grep to print lines that match
            if (close(pipetwo[1]) == -1) {
                fprintf(stderr, "Error with close on pipetwo[1] in grep child: %s\n", strerror(errno));
            }
            if (close(pipetwo[0]) == -1) {
                fprintf(stderr, "Error with close on pipetwo[0] in grep child: %s\n", strerror(errno));
            }
            grep(pattern);
        }

        child_more = fork();
        if(child_more == 0) {
            if (close(pipeone[1]) == -1) {
                fprintf(stderr, "Error with close on pipeon[1] in more child: %s\n", strerror(errno));
            }
            if (close(pipeone[0]) == -1) {
                fprintf(stderr, "Error with close on pipeone[0] in more child: %s\n", strerror(errno));
            }
            if (dup2(pipetwo[0], STDIN_FILENO) == -1) {
                fprintf(stderr, "Error with dup2 on pipetwo[0] in more child: %s\n", strerror(errno));
            }
            if (close(pipetwo[1]) == -1) {
                fprintf(stderr, "Error with close on pipetwo[1] in more child: %s\n", strerror(errno));
            }
            if (close(pipetwo[0]) == -1) {
                fprintf(stderr, "Error with close on pipetwo[0] in more child: %s\n", strerror(errno));
            }
            more();
        }

        while(1) {
            read_bytes = read(fd, buf, 4096);
            if(read_bytes == -1) {
                fprintf(stderr, "Error reading from infile: %s/n", strerror(errno));
                exit(EXIT_FAILURE);
            } else if(read_bytes == 0){
                close(fd);
                close(pipeone[1]);
                close(pipeone[0]); //close the pipe everywhere including child to reach EOF condition
                break; //only break condition of while loop.
            }
            else {
            // instead of byte_count do write_size because we're trying to check the armount of bytes written and compare it to the bytes read
                write_bytes = write(pipeone[1], buf, read_bytes);
                if (write_bytes == -1) {
                    //check errno is ENINTR if it continue
                    if(errno == EINTR) {
                        continue;
                    }
                    fprintf(stderr, "Error with write to outpipe\n");
                    exit(EXIT_FAILURE);
                }
                else if (write_bytes < read_bytes) { //partial write
                    partial_write(read_bytes, write_size, buf, fd);
                }
                else {
                    byte_count += write_bytes;
                }
            }
        }
        if(wait(&wstatus) == -1) {
            fprintf(stderr, "Error with wait: %s\n", strerror(errno));
        }
        if(close(pipetwo[1]) == -1) {
            fprintf(stderr, "Error closing pipetwo[1] at end of processing: %s\n", strerror(errno));
        }//close every pipe each file processed
        if(close(pipetwo[0]) == -1) {
            fprintf(stderr, "Error closing pipetwo[0] at end of processing: %s\n", strerror(errno));
        }
    }
}