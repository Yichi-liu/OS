#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/errno.h>
//#include <linux/limits.h>


char *readline(char *line, FILE *stream);
int buildcommands( char *buf, char *token);
int redirection(char *token);
int child(char *comarg, char *token, const char *space);

int main(int argc, char *argv[]){
    char *buf = NULL;
    int exitval;
    if(argc == 1){ //reading line from stdin
        while(1){
            buf = readline(buf, stdin);
            if(buf == NULL){
                fprintf(stderr,"buf is wrong");
                return -1;
            }

            if(buf[0] != '#'){ //real command
                const char *space = " ";
                char comarg[ARG_MAX];
                strcpy(comarg,buf);
                char *token = strtok(buf,space);
                int builtc = buildcommands(buf,token);
                if(builtc == -1 || builtc == 0){
                    //reads the next line.
                } else {
                    exitval = child(comarg, token, space);
                }
            }
        }
    } else {//this means shell is being called as an interpreter. Not reading from stdin

        FILE *fp;        
        fp = fopen(argv[1], "r");
        if(fp == NULL){
            perror("something wrong opening this shell script");
            exit(EXIT_FAILURE);
        }
        
        while(1){
            buf = readline(buf, fp);
            printf("%s",buf);
            if(buf == 0){ //EOF
                fclose(fp);
                fprintf(stderr,"end of file read, exiting shell with exit code %d\n", exitval);
                exit(exitval);
            } else if(buf == NULL){
                return -1;
            }

            if(buf[0] != '#'){ //real command
                const char *space = " ";
                char comarg[ARG_MAX];
                strcpy(comarg,buf);
                char *token = strtok(buf,space);
                int builtc = buildcommands(buf,token);
                if(builtc == -1 || builtc == 0){
                    //reads the next line. if the command was a builtin command or error.
                } else {
                    exitval = child(comarg, token, space);
                }
            }  
        }
    }
    free(buf);
    return 0;
}

char *readline(char *line, FILE *stream){
    ssize_t lineread;
    size_t len = 0;

    lineread = getline(&line, &len, stream);
    if(lineread == -1 ){
        free(line);
        
        if(errno == 0){ //no errors
            return 0;
        }

        perror("Something wrong reading the line/input derp derp");
        return NULL; //exits any error    
    }
    return line;
}

int buildcommands( char *buf, char *token){
    char command[ARG_MAX];
    const char *space = " ";
    sscanf(buf,"%s",command);
    if(strcmp(command,"cd") == 0 || strcmp(command,"pwd") == 0 || strcmp(command,"exit") == 0){
        if(strcmp(command,"cd") == 0){
            char *path;
            token = strtok(NULL,space);
            if(token == NULL){
                path = getenv("HOME");
            } else {
                token[strlen(token) - 1] = '\0'; //get rid of the new line character
                path = token;
            }
            if(chdir(path) != 0){
                perror("Can't open directory");
                return -1;
            }
        } if(strcmp(command,"pwd") == 0){
            char cwd[PATH_MAX]; //I got this online
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("Current working dir: %s\n", cwd);
            } else {
                perror("something wrong to display the current working directory");
                return -1;
            }
        } if(strcmp(command,"exit") == 0){
            char *value;
            int status;
            token = strtok(NULL,space);
            if(token == NULL){
                int wstatus;
                wait(&wstatus);
                status = WEXITSTATUS(wstatus);
            } else {
                token[strlen(token) - 1] = '\0'; //get rid of the null terminating character
                value = token;
                status = atoi(value);
            }
            exit(status);
        }
        return 0;
    } else {
        return 1; //not a built in command
    }
    
}

int redirection(char *token){ //return 1 means a redirection happened
    if(token[0] == '<' || token[0] == '>' || token[0] == '2'){
        if(token[strlen(token) - 1] == '\n'){
            token[strlen(token) - 1] = '\0'; //in case if there is a new line
        }
        int file;
        char *token2 = malloc(ARG_MAX + 1);
        for(int i = 0; token[i] != '\0'; i++){
            token2[i]=token[i];
        }
        token = strtok(NULL," ");
        if(token != NULL){
            if(token[0] == '<' || token[0] == '>' || token[0] == '2'){
                int again = redirection(token);
                if(again == -1){
                    exit(1);
                }
            }
        }
        if(token2[0] == '2'){
            if(token2[1] == '>'){
                if(token2[2] == '>'){
                    //open/create/append, stderr
                    token2 = strtok(token2,"2>>");
                    file = open(token2, O_RDWR | O_CREAT | O_APPEND, 0666);
                } else {
                    //open/create/trun,stderr
                    token2 = strtok(token2,"2>");
                    file = open(token2, O_RDWR | O_CREAT | O_TRUNC, 0666);
                }
                if(file == -1){
                    perror("something wrong opening");
                    return -1;
                }

                int file2 = dup2(file, STDERR_FILENO);
                if(file2 == -1){
                    perror("something wrong duplicating");
                    return -1;
                }
                close(file);
                return 1;
            } else {
                return 0; //not a redirection operation just happens to have 2 as first letter.
            }
        } if(token2[0] == '<'){
            //open,stdin
            token2 = strtok(token2,"<");
            file = open(token2, O_RDWR);
            if(file == -1){
                perror("something wrong opening");
                return -1;
            }

            int file2 = dup2(file, STDIN_FILENO);
            if(file2 == -1){
                perror("something wrong duplicating");
                return -1;
            }
            close(file);
            return 0; //this is simply reading the file and it is usally used as one of the arguments
        } if(token2[0] == '>'){
            if(token2[1] == '>'){
                //open/create/trun, stdout
                token2 = strtok(token2,">>");
                file = open(token2, O_RDWR | O_CREAT | O_TRUNC, 0666);
            } else {
                //open/create/append, stdout
                token2 = strtok(token2,">");
                file = open(token2, O_RDWR | O_CREAT | O_APPEND, 0666);
            }

            if(file == -1){
                perror("something wrong opening");
                return -1;
            }

            int file2 = dup2(file, STDOUT_FILENO);
            if(file2 == -1){
                perror("something wrong duplicating");
                return -1;
            }
            close(file);
            return 1;                               
        }
    }
    return 0; //not redirection
}

int child(char *comarg, char *token, const char *space){
    int ws;
    int pid = fork();
    if (pid == -1){
        perror("Something wrong calling fork");
    } else if (pid == 0){
        int i = 0; //counts how many arguments + command there are not including redirection operations
        while(token != NULL){
            int roperation = redirection(token);
            if(roperation == -1){
                exit(1);
            }
            if(roperation == 0){
                i++;
            }
            if(roperation == 1){
                break;
            }
            token = strtok(NULL,space);
        }
        char *argument = strtok(comarg,space);
        char **arg = malloc((i + 1)*sizeof(char *)); //found it online
        arg[0] = argument; //this is the command
        if(i == 1){//no arguments
            argument[strlen(argument) - 1] = '\0'; //getting rid of new line again
        }                  
        for(int j=1;j<i;j++){
            argument = strtok(NULL,space);
            if(argument[strlen(argument) - 1] == '\n'){ 
                argument[strlen(argument) - 1] = '\0';
            }
            if(argument[0] == '<'){
                argument = strtok(NULL,"<");
            }
            arg[j] = argument;
        }
        arg[i] = NULL;
        int exe = execvp(arg[0],arg);
        if(exe == -1){
            perror("Something wrong executing the command");
            free(arg);
            exit(127);
        }
        free(arg);
        exit(0);
    }
    pid_t childPID;
    childPID = waitpid(pid, &ws, 0);
    int exitStat = -1;
    if(WIFEXITED(ws)){
        exitStat = WEXITSTATUS(ws); //child exit status
        if(exitStat == 0){ 
            fprintf(stderr,"Child Process %d exited normally\n", childPID); 
        } else {
            fprintf(stderr,"Child Process %d exited with return value %d\n",childPID,exitStat);
        }
    }
    
    struct tms buf;
    int ticPerS = sysconf(_SC_CLK_TCK);
    clock_t time;
    time = times(&buf);
    if(time == -1){
        perror("something wrong with times sys call");
    }
    fprintf(stderr,"Real: %fs User: %fs Sys: %fs\n",buf.tms_utime/ticPerS,buf.tms_cutime/ticPerS,buf.tms_cstime/ticPerS);
    return exitStat;
}