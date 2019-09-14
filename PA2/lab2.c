 #include <stdio.h>             // for I/O
#include <stdlib.h>            // for I/O
#include <libgen.h>            // for dirname()/basename()
#include <string.h>

typedef struct node{
          char  name[64];       // node's name string
          char  type;
   struct node *child, *sibling, *parent;
}NODE;


NODE *root, *cwd, *start;
char line[128];
char command[16], pathname[64];

//               0       1        2     3     4        5     6     7      8         9
char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "creat", "rm", "pwd","save", "reload", "quit", 0};

//space to hold dirname and basename
char dname[64], bname[64];

int findCmd(char *command)
{
   int i = 0;
   while(cmd[i]){
     if (strcmp(command, cmd[i])==0)
         return i;
     i++;
   }
   return -1;
}

NODE *search_child(NODE *parent, char *name)
{
  NODE *p;
  p = parent->child;
  if (p==0)
    return 0;
  while(p){
    if (strcmp(p->name, name)==0)
      return p;
    p = p->sibling;
  }
  return 0;
}

int insert_child(NODE *parent, NODE *q)
{
  NODE *p;
  printf("insert NODE %s into parent child list\n", q->name);
  p = parent->child;
  if (p==0)
    parent->child = q;
  else{
    while(p->sibling)
      p = p->sibling;
    p->sibling = q;
  }
  q->parent = parent;
  q->child = 0;
  q->sibling = 0;
}

int dbname(char *pathname)
{
  char temp[128]; // dirname(), basename() destroy original pathname strcpy(temp, pathname);
  strcpy(temp, pathname);
  strcpy(dname, dirname(temp));
  strcpy(temp, pathname);
  strcpy(bname, basename(temp));
}

int tokenize(char *pathname, char **path)
{
    char *s;
    int i = 0;
    s = strtok(pathname, "/"); // first call to strtok() 
    while(s){
        path[i] = s;
        s = strtok(0, "/"); // call strtok() until it returns NULL 
        i++;
    }
}


/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname


  //General Procedure:
  //  pathname -> (dname, bname); temp = dname
  //  dname[0] ? root : cwd
  //  path = tokenize(temp); == ["a", "b", "c"]
  //  loop path
  //    start = search(start, path[i])
  //  checks
  //  insert


****************************************************************/

int mkdir(char *pathname)
{
  NODE *p, *q;
  char temp[64];
  char *path[128] = {{0}};


  dbname(pathname);
  strcpy(temp, dname);

  printf("dname = %s\nbname = %s\ntemp = dname = %s\n", dname, bname, temp);
  
  if (!strcmp(bname, "/") || !strcmp(bname, ".") || !strcmp(bname, "..")){
    printf("can't mkdir with %s\n", bname);
    return -1;
  }

  if (dname[0]=='/')
    start = root;
  else
    start = cwd;


  printf("start = %s\n", start->name);
  

  tokenize(temp, path); //to preserve original pathname string


  int j = 0;
  for(j;j < 5; j++){
    if(path[j])
      printf("Token %d: %s\n", j, path[j]);
  }

  printf("while loop:\n");

  int i = 0;
  while(path[i]){
    if(!strcmp(path[i], ".")){
      start = cwd;
      break;
    }
    else{
      printf("\tstart = %s, token = %s\n", start->name, path[i]);
      start = search_child(start, path[i++]);
    }
  }

  if(!start){
    printf("Error: Invalid Path\n");
    return -1;
  }
 
  printf("check whether %s already exists\n", bname);
  p = search_child(start, bname);
  if (p){
    printf("name %s already exists, mkdir FAILED\n", bname);
    return -1;
  }


  printf("--------------------------------------\n");
  printf("ready to mkdir %s\n", bname);
  q = (NODE *)malloc(sizeof(NODE));
  q->type = 'D';
  strcpy(q->name, bname);
  insert_child(start, q);
  printf("mkdir %s OK\n", bname);
  printf("--------------------------------------\n");
    
  return 0;
}

/***************************************************************
//rmdir removes a directory

//General Procedure:
  //  pathname -> (dname, bname); temp = dname
  //  dname[0] ? root : cwd
  //  path = tokenize(temp); == ["a", "b", "c"]
  //  loop path
  //    start = search(start, path[i])
  //  check existance

***************************************************************/
int rmdir(char *pathname)
{
  NODE *p, *q, *last;
  char temp[64];
  char *path[128] = {{0}};

  dbname(pathname);
  strcpy(temp, dname);

  printf("dname = %s\nbname = %s\ntemp = dname = %s\n", dname, bname, temp);
  
  if (!strcmp(bname, "/") || !strcmp(bname, ".") || !strcmp(bname, "..")){
    printf("can't mkdir with %s\n", bname);
    return -1;
  }

  if (dname[0]=='/')
    start = root;
  else
    start = cwd;


  printf("start = %s\n", start->name);
  

  tokenize(temp, path); //to preserve original pathname string


  int j = 0;
  for(j;j < 5; j++){
    if(path[j])
      printf("Token %d: %s\n", j, path[j]);
  }

  printf("while loop:\n");

  int i = 0;
  while(path[i]){
    if(!strcmp(path[i], ".")){
      start = cwd;
      break;
    }
    else{
      printf("\tstart = %s, token = %s\n", start->name, path[i]);
      start = search_child(start, path[i++]);
    }
  }

  NODE *list = start->child;
  NODE *list_temp = list;

  if(!list){
    printf("Error: Dir Does Not Exist\n");
    return -1;
  }
  if(list->child){
    printf("Error: Dir is not empty\n");
    return -1;
  }
  if(!strcmp(list->name, bname) && list->type == 'D'){
    printf("Deleting from Head\n");
    printf("head = %s, next = NULL or next node\n", list->name);

    list = list->sibling;
    start->child = list;
    free(list_temp);

    printf("newHead = %s\n", list->name);

    return 0;
  }
  else{
    list_temp = list;
    list = list->sibling;
  }

  while(list){
    if(!strcmp(list->name, bname)){
      list_temp->sibling = list->sibling;
      free(list);

      return 0;
    }
    list_temp = list;
    list = list->sibling;
  }

  printf("Error: Dir Does Not Exist\n");
  

  // printf("check whether %s already exists\n", bname);
  // p = search_child(start, bname);
  // if (p->type != 'D'){
  //   printf("name %s is not Dir type FAILED\n", bname);
  //   return -1;
  // }


  // printf("--------------------------------------\n");
  // last->sibling = start->sibling;
  // free(start);
  // start = last;

  // printf("--------------------------------------\n");
    
  return -1;
}

// This ls() list CWD. You MUST improve it to ls(char *pathname)
int ls(char *pathname)
{
  NODE *p = cwd->child; //default

  if(pathname[0]){
        char temp[64];
        char *path[128] = {{0}};

        strcpy(temp, pathname);
        tokenize(temp, path);

        if (pathname[0]=='/')
            p = root;
        else
            p = cwd;

        int i = 0;
        while(p &&path[i])
          p = search_child(p, path[i++]);
        

        if(!p || p->type != 'D'){
          printf("Error: Invalid Path\n");
          return -1;
        }
        else
          p = p->child;
  }

  if(p == NULL){
    printf("Dir is Empty\n");
    return;
  }

  printf("%s contents = ", p->parent->name);
  while(p){
    printf("[%c %s] ", p->type, p->name);
    p = p->sibling;
  }
  printf("\n");

}

//cd changes directory
int cd(char *pathname)
{
  NODE *p;
  char temp[64];
  char *path[128] = {{0}};

  if(pathname[0] == NULL){
    printf("Error: Enter a path\n");
    return -1;
  }

  strcpy(temp, pathname);
  tokenize(temp, path);

  if (pathname[0]=='/')
    p = root;
  else
    p = cwd;

  int i = 0;
  while(p &&path[i]){
    if(!strcmp(path[i], "..")){
      p = p->parent;
      i++;
    }
    else
      p = search_child(p, path[i++]);
  }

  if(p != NULL && p->type == 'D')
    cwd = p;
  else
    printf("Invalid path\n");

}

//creat makes a file
int creat(char *pathname)
{
  NODE *p, *q;
  char temp[64];
  char *path[128] = {{0}};


  dbname(pathname);
  strcpy(temp, dname);

  printf("dname = %s\nbname = %s\ntemp = dname = %s\n", dname, bname, temp);
  
  if (!strcmp(bname, "/")){
    printf("can't creat file with /\n");
    return -1;
  }

  if (dname[0]=='/')
    start = root;
  else
    start = cwd;


  printf("start = %s\n", start->name);
  

  tokenize(temp, path); //to preserve original pathname string


  int j = 0;
  for(j;j < 5; j++){
    if(path[j])
      printf("Token %d: %s\n", j, path[j]);
  }

  printf("while loop:\n");

  int i = 0;
  while(path[i]){
    if(!strcmp(path[i], ".")){
      start = cwd;
      break;
    }
    else{
      printf("\tstart = %s, token = %s\n", start->name, path[i]);
      start = search_child(start, path[i++]);
    }
  }

  if(!start){
    printf("Error: Invalid Path\n");
    return -1;
  }
 
  printf("check whether %s already exists\n", bname);
  p = search_child(start, bname);
  if (p){
    printf("name %s already exists, creat FAILED\n", bname);
    return -1;
  }


  printf("--------------------------------------\n");
  printf("ready to creat %s\n", bname);
  q = (NODE *)malloc(sizeof(NODE));
  q->type = 'F';
  strcpy(q->name, bname);
  insert_child(start, q);
  printf("creat %s OK\n", bname);
  printf("--------------------------------------\n");
    
  return 0;
}

//rm deletes a file
int rm(char *pathname)
{
  NODE *p, *q, *last;
  char temp[64];
  char *path[128] = {{0}};

  dbname(pathname);
  strcpy(temp, dname);

  printf("dname = %s\nbname = %s\ntemp = dname = %s\n", dname, bname, temp);
  
  if (!strcmp(bname, "/")){
    printf("can't rm /\n");
    return -1;
  }

  if (dname[0]=='/')
    start = root;
  else
    start = cwd;


  printf("start = %s\n", start->name);
  

  tokenize(temp, path); //to preserve original pathname string


  int j = 0;
  for(j;j < 5; j++){
    if(path[j])
      printf("Token %d: %s\n", j, path[j]);
  }

  printf("while loop:\n");

  int i = 0;
  while(path[i]){
    if(!strcmp(path[i], ".")){
      start = cwd;
      break;
    }
    else{
      printf("\tstart = %s, token = %s\n", start->name, path[i]);
      start = search_child(start, path[i++]);
    }
  }

  NODE *list = start->child;
  NODE *list_temp = list;

  if(!list){
    printf("Error: File Does Not Exist\n");
    return -1;
  }
  if(!strcmp(list->name, bname) && list->type == 'F'){
    printf("Deleting from Head\n");
    printf("head = %s, next = NULL or next node\n", list->name);

    list = list->sibling;
    start->child = list;
    free(list_temp);

    printf("newHead = %s\n", list->name);

    return 0;
  }
  else{
    list_temp = list;
    list = list->sibling;
  }

  while(list){
    if(!strcmp(list->name, bname)){
      list_temp->sibling = list->sibling;
      free(list);

      return 0;
    }
    list_temp = list;
    list = list->sibling;
  }

  printf("Error: File Does Not Exist\n");
  

  // printf("check whether %s already exists\n", bname);
  // p = search_child(start, bname);
  // if (p->type != 'D'){
  //   printf("name %s is not Dir type FAILED\n", bname);
  //   return -1;
  // }


  // printf("--------------------------------------\n");
  // last->sibling = start->sibling;
  // free(start);
  // start = last;

  // printf("--------------------------------------\n");
    
  return -1;
}

//pwdHelper
int pwdHelper(NODE *node, int saveMode, FILE *out){
  if(!strcmp(node->name, "/")){
    if(saveMode)
      fprintf(out, "/");
    else
      printf("/");
    return;
  }
  else
    pwdHelper(node->parent, saveMode, out);

  if(saveMode)
    fprintf(out, "%s/", node->name);
  else
    printf("%s/", node->name);

  return 0;
}
//pwd prints the working directory
int pwd(int mode, NODE* start, FILE *out){
  if(mode)
    pwdHelper(start, mode, out);
  else
    pwdHelper(cwd, mode, out);
  

  if(mode)
    fprintf(out,"\n");
  else
    printf("\n");
  
  return 0;
}

//saveHelper
int saveHelper(NODE *node, FILE *out){
  if(!node)
    return;

  fprintf(out, "%c", node->type);
  pwd(1, node, out);
  saveHelper(node->child, out);
  saveHelper(node->sibling, out);
}
//save saves the filesystem as a textfile
int save(){
  FILE *out = fopen("save.txt", "w+");
  saveHelper(root, out);
  fclose(out);

  return 0;
}
//clearTreeHelper
int clearTreeHelper(NODE* node){
  if(!node){
    return;
  }
  else{
  clearTreeHelper(node->child);
  clearTreeHelper(node->sibling);

  node->sibling = NULL;
  node->child = NULL;
  node->parent = NULL;
  free(node);
  }

  return 0;
}
//clearTree frees all memory of tree
int clearTree(){
  clearTreeHelper(root);
}

//reload reads from a file and populates the tree
int reload(){
  FILE *in = fopen("save.txt", "r+");
  if(!in){
    printf("Error: Cant open save file\n");
    return -1;
  }
  clearTree();
  initialize();
  
  char line[128];
  char type;
  char pathname[127];

  fgets(line, sizeof(line),in); // get rid of root definition

  while(1){
    fgets(line, sizeof(line),in);
    if(feof(in))
      break;
    sscanf(line, "%c%s\n", &type, pathname);

    printf("type = %c, path = %s\n", type, pathname);
    getchar();

    if(type == 'D')
      mkdir(pathname);
    else
      creat(pathname);
    
  }



  fclose(in);
  return 0;
}

int quit()
{
  printf("Program exit\n");
  save();
  exit(0);
  // improve quit() to SAVE the current tree as a Linux file
  // for reload the file to reconstruct the original tree
}

int initialize()
{
    root = (NODE *)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = 0;
    root->child = 0;
    root->type = 'D';
    cwd = root;
    printf("Root initialized OK\n");
}

int main()
{
  int index;

  initialize();

  printf("NOTE: commands = [mkdir|ls|quit]\n");

  while(1){
      printf("Enter command line : ");
      fgets(line, 128, stdin);
      line[strlen(line)-1] = 0;

      command[0] = pathname[0] = 0;
      sscanf(line, "%s %s", command, pathname);
      printf("command=%s pathname=%s\n", command, pathname);
      
      if (command[0]==0) 
         continue;

      index = findCmd(command);

      switch (index){
        case 0: mkdir(pathname); break;
        case 1: rmdir(pathname); break;
        case 2: ls(pathname);    break;
        case 3: cd(pathname);    break;
        case 4: creat(pathname); break;
        case 5: rm(pathname);    break;
        case 6: pwd(0, 0, 0);       break;
        case 7: save();          break;
        case 8: reload();        break;
        case 9: quit();          break;
      }
  }
}

