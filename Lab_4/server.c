#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libgen.h>

#include <unistd.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>


#define MAX   512
#define PORT 1234
  
char line[MAX], *tokens[MAX];
int n, r, size = 0;
char cwd[128], cmd[MAX], filename[MAX];

int logic()
{
    printf("tk1 = %s, tk2 = %s\n", cmd, filename);
    if(!strcmp("pwd", cmd))
    {
        strcpy(line, cwd);
    }
    else if(!strcmp("ls", cmd))
    {
        if(!strcmp(".", filename))
        {
            printf("HERE\n");
            ls(cwd);
        }
        else if (!strcmp("..", filename))
        {
            // ls(dirname(cwd));
            char temp[64];
            strcpy(temp, tokens[1]);

            printf("dirname = %s\n", dirname(temp));
        }
    }
}

int main(int argc, char *argv[]) 
{ 
    int sfd, cfd, len; 
    struct sockaddr_in saddr, caddr; 
  
    printf("1. create a TCP socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sfd < 0) { 
        printf("socket creation failed\n"); 
        exit(1); 
    }

    printf("2. fill in [localhost IP port=1234] as server address\n");
    bzero(&saddr, sizeof(saddr)); 
    saddr.sin_family = AF_INET; 
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    saddr.sin_port = htons(PORT); 
  
    printf("3. bind socket with server address\n");
    if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0) { 
        printf("socket bind failed\n"); 
        exit(2); 
    }
      
    printf("4. server listens\n");
    if ((listen(sfd, 5)) != 0) { 
        printf("Listen failed\n"); 
        exit(3); 
    }

    len = sizeof(caddr);
    while(1){
       printf("server accepting connection\n");
       cfd = accept(sfd, (struct sockaddr *)&caddr, &len); 
       if (cfd < 0) { 
          printf("server acccept failed\n"); 
          exit(4); 
       }
       printf("server acccepted a client connection\n"); 

       // server Processing loop
       while(1){
            printf("server: ready for next request\n");
	        n = read(cfd, line, MAX);
            if (n==0){
                printf("server: client died, server loops\n");
                close(cfd);
                break;
            }

            printf("Server receives: \n");
            printf("raw = %s\n", line);

            getcwd(cwd, MAX);
            tokenize();
            
            printf("CWD = %s\n", cwd);
            printf("First token = %s\n", tokens[0]);
            
            printf("Second token = %s\n", tokens[1]);
      
            logic();

            // send the echo line to client 
            n = write(cfd, line, MAX);
            memset(line, 0, strlen(line));
            //printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
       }
    }
} 