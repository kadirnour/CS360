#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>


#define MAX   512
#define PORT 1234

char ans[MAX], cwd[MAX], *tokens[MAX], line[MAX];
int n, m, size;


int main(int argc, char *argv[ ]) 
{ 
    int cfd; 
    struct sockaddr_in saddr; 
  
    printf("1. create a TCP socket\n");
    cfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (cfd < 0) { 
        printf("socket creation failed\n");
        exit(1);
    }

    printf("2. fill in [localhost IP port=1234] as server address\n");
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(PORT);
  
    printf("3. connect client socket to server\n");
    if (connect(cfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) { 
        printf("connection to server failed\n"); 
        exit(2); 
    }
    
    printf("********  processing loop  *********\n");
    while(1){
        bzero(line, MAX);
        clearTok();
        size = 0;

        printf("server: ready for next request\n");
        printf("********************** menu *********************\n");
        printf("* get  put  ls   cd   pwd   mkdir   rmdir   rm  *\n");
        printf("* lcat     lls  lcd  lpwd  lmkdir  lrmdir  lrm  *\n");
        printf("*************************************************\n");
        
        fgets(line, MAX, stdin);
        line[strlen(line)-1] = 0;        // kill \n at end
        if (line[0]==0)                  // exit if NULL line
            exit(0);
        
        // tokenize();
        // getcwd(cwd, 128);

        // printf("First token = %s\n", tokens[0]);
        // printf("Second token = %s\n", tokens[1]);

        // char dname[MAX];
        // strcpy(dname, cwd);
        // printf("dname = %s\n", dname);

        // ls(dname);

        n = write(cfd, line, MAX);
        memset(line, 0, strlen(line));


        // if(!strcmp("pwd", tokens[0]))
        // {
        //     n = write(cfd, tokens, MAX);
        // }
        // else if(!strcmp("ls", tokens[0]))
        // {
        //     n = write(cfd, tokens, MAX);
        // }

       // Send ENTIRE line to server
       //n = write(cfd, line, MAX);

       // Read a line from sock and show it
        n = read(cfd, ans, MAX);
        printf("RESULT:\n\t%s\n", ans);
       // printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
       
    }
}