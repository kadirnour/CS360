#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

char cmd[128];

int token(char *line, char **path, int *size, char *op){
    char *s;
    int i = 0;
    s = strtok(line, op); // first call to strtok() 
    while(s){
        path[i] = s;
        s = strtok(0, op); // call strtok() until it returns NULL 
        i++;
    }

    *size = i;
}
void redirection(char * args[], int args_size, int i){
    int fd;
    char *op = args[i];
    char *out = args[i + 1];
    if(!strcmp(op, "<")){
        close(0); fd = open(out, O_RDONLY);
    }
    else if(!strcmp(op, ">")){
        close(1); fd = open(out, O_WRONLY|O_CREAT, 0644);
    }
    else if(!strcmp(op, ">>")){
        close(1); fd = open(out, O_WRONLY|O_APPEND|O_CREAT, 0644);
    }
}
void headTail(char *args[], char *head[], char *tail[], int i){
    int j = 0;
    while(i != j){
        strcpy(head[i], args[i++]);
    }
    // while(args[i]){
    //     strcpy(tail[i], args[i++]);
    // }
}
void do_fork(int bins_size, char *bins[], int args_size, char *args[], char *env[], int *status){
    int i = 0, op = 0;
    int pid = fork();
    char *head[50], *tail[50];

    

    if(pid == -1){
        perror("Couldn't do it boss\n");
    }

    if(pid > 0){ //parent
        pid = wait(&status);
    }
    else{ //child
        for(i = 0; i < args_size; i++){
            if(!strcmp(args[i], "<") || !strcmp(args[i], ">") || !strcmp(args[i], ">>")){
                redirection(args,args_size, i);
                args[i +1] = 0;
                args[i] = 0;
                break;
            }
        }
        

        for(i = 0; i < bins_size; i++){

            strcpy(cmd, bins[i]);
            strcat(cmd, "/");
            strcat(cmd, args[0]);

            int r = execve(cmd, args, env);
        }
    }
}
int main (int argc, char *argv[], char *env[]){
    char *PATH = getenv("PATH");
    char input[128];
    char *bins[128], *args[100];
    int status, bins_size, args_size = 0, i;

    // list of bins needed to traverse
    token(PATH, bins, &bins_size, ":"); 

    while(1){

        printf("mysh-- ");
        fgets(input, 128, stdin);
        input[strlen(input)-1] = 0;

        token(input, args, &args_size, " ");
        args[args_size] = 0;

        
        if(!strcmp("cd", args[0])){
            args[1] ? chdir(args[1]) : chdir(".");
        }
        else if(!strcmp("exit", args[0])){
            exit(1);
        }

        else{
            do_fork(bins_size, bins, args_size, args, env, &status);
        }

    }
    return 0;
}