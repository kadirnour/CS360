/************* cd_ls_pwd.c file **************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char   gpath[256];
extern char   *name[64];
extern int    n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];

#define OWNER  000700
#define GROUP  000070
#define OTHER  000007

change_dir()
{
  int ino = getino(pathname);

  printf("dev=%d ino=%d\n", dev, ino);
  MINODE *mip = iget(dev, ino);
  if(!S_ISDIR(mip->INODE.i_mode)) {printf("%s not a DIR\n", pathname); return;}

  iput(running->cwd);
  running->cwd = mip;
}

int ls_file(DIR *dp)
{
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";

  int ino = dp->inode, i;
  char my_time[25];
  MINODE *mip = iget(dev, ino);
  strcpy(my_time, ctime(&mip->INODE.i_atime));

  my_time[strlen(my_time) - 1] = 0;

  if(S_ISDIR(mip->INODE.i_mode)) printf("d");
  if(S_ISREG(mip->INODE.i_mode)) printf("-");
  if(S_ISLNK(mip->INODE.i_mode)) printf("l");

  for (i=8; i >= 0; i--){
    if (mip->INODE.i_mode & (1 << i))
      printf("%c", t1[i]);
    else
      printf("%c", t2[i]);
  }

  printf(" %d %d %d %s %d %s\n", mip->INODE.i_links_count, 
    mip->INODE.i_uid, mip->INODE.i_gid, my_time, mip->INODE.i_size, dp->name);
}

int ls_dir()
{
  if(!strlen(pathname))
    strcpy(pathname, ".");
  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);

  int i;
  char *cp, ibuf[BLKSIZE];

  for(i=0; i<12; i++){
    if(mip->INODE.i_block[i] == 0)
      return 0;
    get_block(mip->dev, mip->INODE.i_block[i], ibuf);
    dp = (DIR *)ibuf;
    cp = ibuf;

    while(cp < ibuf + BLKSIZE){
      //go to inode and print stats
      ls_file(dp);

      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
}

int list_file()
{
  ls_dir();
}

void rpwd(MINODE *wd)
{
  char my_name[128];
  int my_ino, parent_ino;
  if(wd == root) return;
  my_ino = get_myino(wd, &parent_ino);

  MINODE *pip = iget(dev, parent_ino);
  get_myname(pip, my_ino, my_name);

  rpwd(pip);
  printf("/%s", my_name);
}


int pwd(MINODE *wd)
{
  if(wd == root) printf("/");
  else           rpwd(wd);
  printf("\n");
}



