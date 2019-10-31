/************* cd_ls_pwd.c file **************/
// #include "type.h"
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
  if(!ino)
    return -1;
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

int enter_name(MINODE *pip, int ino, char *name)
{
  int i, ideal, remain, needed, added = 0;
  char buf[BLKSIZE], *cp;

  for(i=0; i<12; i++){
    if(pip->INODE.i_block[i] == 0) break;

    get_block(pip->dev, pip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;
    
    while(cp + dp->rec_len < buf + BLKSIZE){
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
    
    ideal = 4 *((8 + dp->name_len + 3)/4);
    remain = dp->rec_len - ideal;
    needed = 4 *((8 + strlen(name) + 3)/4);

    if(remain >= needed){
      dp->rec_len = ideal;
      cp += dp->rec_len;
      dp = (DIR *)cp;

      dp->inode = ino;
      strcpy(dp->name, name);
      dp->name_len = strlen(dp->name);
      dp->rec_len = remain;

      added = 1;
    }

    put_block(pip->dev, pip->INODE.i_block[i], buf);
  }
  if(!added){
    int blk = balloc(pip->dev);
    if(!blk) return -1;

    pip->INODE.i_block[i] = blk;
    pip->INODE.i_size += BLKSIZE;
    get_block(pip->dev, blk, buf);
    dp = (DIR *)buf;

    dp->inode = ino;
    strcpy(dp->name, name);
    dp->name_len = strlen(dp->name);
    dp->rec_len = BLKSIZE;

    put_block(pip->dev, blk, buf);
  }

  return 0;
}

int mymkdir(MINODE *pip, char *child)
{
  char buf[BLKSIZE], *cp;
  int ino, blk, i;
  MINODE *mip;

  ino = ialloc(dev);
  blk = balloc(dev);

  mip = iget(dev, ino);
  ip = &mip->INODE;
  ip->i_mode = 0x41ED;
  ip->i_uid = running->cwd->INODE.i_uid;
  ip->i_gid = running->cwd->INODE.i_gid;
  ip->i_size = BLKSIZE;
  ip->i_links_count = 2;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2;
  ip->i_block[0] = blk;
  for(i=0; i<13; i++){ip->i_block[i+1] = 0;}
  mip->dirty = 1;
  iput(mip);

  bzero(buf, BLKSIZE);
  dp = (DIR *)buf;
  dp->inode = ino;
  dp->rec_len = 12;
  dp->name_len = 1;
  dp->name[0] = '.';

  dp = (char *)dp + 12;

  dp->inode = pip->ino;
  dp->rec_len = BLKSIZE-12;
  dp->name_len = 2;
  dp->name[0] = dp->name[1] = '.';

  put_block(dev, blk, buf);

  enter_name(pip, ino, child);

}

int make_dir()
{
  int pino;
  char parent[128], child[64], temp[128];
  MINODE *start, *pip;
  if(pathname[0] == '/') {start = root; dev = root->dev;}
  else                   {start = running->cwd; dev = running->cwd->dev;}

  strcpy(temp, pathname);
  strcpy(parent, dirname(temp));
  parent[strlen(parent)] = 0;

  strcpy(temp, pathname);
  strcpy(child, basename(temp));
  child[strlen(child)] = 0;

  pino = getino(parent);
  pip = iget(dev, pino);

  if(!S_ISDIR(pip->INODE.i_mode)) {printf("%s Is Not A DIR\n", parent); return;}
  if(search(pip, child)){printf("%s Is Already A DIR\n", child); return;}

  mymkdir(pip, child);

  pip->INODE.i_links_count++;
  pip->dirty = 1;
  iput(pip);

}
int mycreat(MINODE *pip, char *child)
{
  int ino, i;
  MINODE *mip;

  ino = ialloc(dev);

  mip = iget(dev, ino);
  ip = &mip->INODE;
  ip->i_mode = 0x81A4;
  ip->i_uid = running->cwd->INODE.i_uid;
  ip->i_gid = running->cwd->INODE.i_gid;
  ip->i_size = 0;
  ip->i_links_count = 1;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 0;
  for(i=0; i<14; i++){ip->i_block[i] = 0;}
  mip->dirty = 1;
  iput(mip);

  enter_name(pip, ino, child);
}

int creat_file()
{
  int pino;
  char parent[128], child[64], temp[128];
  MINODE *start, *pip;
  if(pathname[0] == '/') {start = root; dev = root->dev;}
  else                   {start = running->cwd; dev = running->cwd->dev;}

  strcpy(temp, pathname);
  strcpy(parent, dirname(temp));
  parent[strlen(parent)] = 0;

  strcpy(temp, pathname);
  strcpy(child, basename(temp));
  child[strlen(child)] = 0;

  pino = getino(parent);
  pip = iget(dev, pino);

  if(!S_ISDIR(pip->INODE.i_mode)) {printf("%s Is Not A DIR\n", parent); return;}
  if(search(pip, child)){printf("%s Is Already A DIR\n", child); return;}

  mycreat(pip, child);

  pip->dirty = 1;
  iput(pip);
}

