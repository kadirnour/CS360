#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>

#define MAX 512
extern char line[MAX];
extern char *tokens[MAX];
extern int size;

int clearTok()
{
    int i = 0;
    for(i; i < size; i++)
    {
        tokens[i] = NULL;
    }
}

int tokenize()
{
    char *s;
    char temp[100];
    strcpy(temp, line);

    s = strtok(temp, " ");
    while(s)
    {
        tokens[size++] = s;
        s = strtok(NULL, " ");
    }
}

int cat(char *pathname)
{
  FILE* fp;
  int c;

  fp = fopen(pathname, "r");

  if(!fp) return -1;
  
  while((c = fgetc(fp)) != EOF) putchar(c);
  
  printf("\n");
  fclose(fp);
  return 0;
}

int cp(char *pathname1, char *pathname2)
{
  int n, total = 0;
  char *buf[4096];
  FILE* fp1 = fopen(pathname1, "r"), *fp2 = fopen(pathname2, "w");

  if(!fp1 || !fp2) return -1;
  
  while(n = fread(buf, 1, 4096, fp1))
  {
    fwrite(buf, 1, n, fp2);
    total += n;
  }
  fclose(fp1); fclose(fp2);
  return 0;
}

int ls_file(char *fname)
{
  char *t1 = "xwrxwrxwr-------"; 
  char *t2 = "----------------";

  struct stat fstat, *sp;
  int r, i;
  char ftime[64];
  sp = &fstat;


  if((r = lstat(fname, &fstat)) < 0) return -1;
  
  if((sp->st_mode & 0xF000) == 0x8000) strcat(line, "-");
  if((sp->st_mode & 0xF000) == 0x4000) strcat(line, "d");


  for(i = 8; i >= 0; i--){
    if (sp->st_mode & (1 << i)){
      line[strlen(line)] = t1[i];

    }
    else{
      line[strlen(line)] = t2[i];
    }
  }

  char temp[64];

  sprintf(temp, "%4d %4d %4d %8d ", sp->st_nlink, sp->st_gid, sp->st_uid, sp->st_size);
  strcat(line, temp);

  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  strcat(line ,ftime);
  strcat(line, " ");
  basename(fname);
  strcat(line, fname);
  strcat(line, "\n");
}

int ls(char *dname)
{
  DIR *dp;
  struct dirent *ep;
  if(!(dp = opendir(dname))) return -1;
  
  memset(line,0,strlen(line));

  while((ep = readdir(dp)))
  {
    ls_file(ep->d_name);
  }
}
