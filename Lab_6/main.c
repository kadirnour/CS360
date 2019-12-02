/****************************************************************************
*                   KCW testing ext2 file system                            *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>

#include "type.h"

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
OFT    openFileTable[NOFT];

char   gpath[256]; // global for tokenized components
char   *name[64];  // assume at most 64 components in pathname
int    n;          // number of component strings

int    gfd, dev;
int    nblocks, ninodes, bmap, imap, inode_start;
char   line[256], cmd[32], pathname[64], pathname2[64];

#include "util.c"
#include "cmd.c"

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;
  OFT    *o;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mountptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
   for (i=0; i<NOFT; i++){
    o = &openFileTable[i];
    o->mode = 0;
    o->refCount = 0;
    o->inodeptr = 0;
    o->offset = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

char *disk = "mydisk";
int main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];
  if (argc > 1)
    disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((gfd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);  exit(1);
  }
  dev = gfd;
  /********** read super block at 1024 ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system *****************/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root();

  printf("root refCount = %d\n", root->refCount);
  
  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  
  printf("root refCount = %d\n", root->refCount);

  //printf("hit a key to continue : "); getchar();
  while(1){
    printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|chmod|utime|quit] ");
    fgets(line, 256, stdin);
    line[strlen(line)-1] = 0;
    if (line[0]==0)
      continue;
    pathname[0] = 0;
    cmd[0] = 0;
    
    sscanf(line, "%s %s %s", cmd, pathname, pathname2);
    printf("cmd=%s pathname=%s\n", cmd, pathname);

    if (strcmp(cmd, "ls")==0)
       list_file();
    if (strcmp(cmd, "cd")==0)
       change_dir();
    if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
    if (strcmp(cmd, "mkdir")==0)
       make_dir();
    if (strcmp(cmd, "creat")==0)
       creat_file();
    if(strcmp(cmd,  "rmdir")==0)
       rm_dir();
    if(strcmp(cmd,  "link")==0)
       link_file();
    if(strcmp(cmd,  "unlink")==0)
       unlink_file();
    if(strcmp(cmd,  "symlink")==0)
       symlink_file();
    if(strcmp(cmd,  "chmod")==0)
       mychmod();
    if(strcmp(cmd,  "utime")==0)
       utime();
    if(strcmp(cmd,  "open")==0)
       open_file();
    if(strcmp(cmd,  "close")==0)
       close_file();
    if(strcmp(cmd,  "lseek")==0)
       lseek_file();
    if(strcmp(cmd,  "read")==0)
       read_file();
    if(strcmp(cmd,  "write")==0)
       write_file();
    if(strcmp(cmd,  "cat")==0)
       cat_file();
    if(strcmp(cmd,  "cp")==0)
       cp_file();
    if(strcmp(cmd,  "pfd")==0)
       pfd();
    if (strcmp(cmd, "test")==0)
       test();
    if (strcmp(cmd, "quit")==0)
       quit();
  }
}
 
int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}
