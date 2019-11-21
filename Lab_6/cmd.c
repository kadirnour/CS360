/************* cd_ls_pwd.c file **************/
// #include "type.h"
/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern OFT    openFileTable[NOFT];
extern char   gpath[256];
extern char   *name[64];
extern int    n;
extern int    gfd, dev;
extern int    nblocks, ninodes, bmap, imap, inode_start;
extern char   line[256], cmd[32], pathname[64], pathname2[64], readbuf[BLKSIZE];

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

  printf(" %d %d %d %s %d %s", mip->INODE.i_links_count, 
    mip->INODE.i_uid, mip->INODE.i_gid, my_time, mip->INODE.i_size, dp->name);
  
  if(S_ISLNK(mip->INODE.i_mode)){
    char linkname[64];
    myreadlink(mip, linkname);
    printf(" -> %s", linkname);
  }

  printf("\n");
  iput(mip);
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
    iput(mip);
  }
}

int list_file()
{
  ls_dir();
}

void rpwd(MINODE *wd, char *absolute)
{
  char my_name[128];
  int my_ino, parent_ino;
  if(wd == root) return;
  my_ino = get_myino(wd, &parent_ino);

  MINODE *pip = iget(dev, parent_ino);
  get_myname(pip, my_ino, my_name);

  rpwd(pip, absolute);
  strcat(absolute, "/");
  strcat(absolute, my_name); 
  iput(pip);
}


int pwd(MINODE *wd)
{
  char absolute[60] = {0};
  if(wd == root) printf("/");
  else           rpwd(wd, absolute);
  printf("%s\n", absolute);
}

int enter_name(MINODE *pip, int ino, char *name)
{
  printf("Entering name %s into parent\n", name);

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

  return ino;
}

int make_dir()
{
  int ino, pino;
  char parent[128], child[64], temp[128];
  MINODE *pip;
  if(pathname[0] == '/') {dev = root->dev;}
  else                   {dev = running->cwd->dev;}

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

  ino = mymkdir(pip, child);

  pip->INODE.i_links_count++;
  pip->dirty = 1;
  iput(pip);

  return ino;
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

  return ino;
}

int creat_file()
{
  int ino, pino;
  char parent[128], child[64], temp[128];
  MINODE *pip;
  if(pathname[0] == '/') {dev = root->dev;}
  else                   {dev = running->cwd->dev;}

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

  ino = mycreat(pip, child);

  pip->dirty = 1;
  iput(pip);

  return ino;
}

int verifyEmpty(MINODE *mip)
{
  int count = 0;
  char buf[BLKSIZE], *cp;
  if(mip->INODE.i_links_count > 2){ return 0;}

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;

  while(cp < buf + BLKSIZE){
    if(count > 2){return 0;}

    count++;
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  return 1;
}

int compactDataBlocks(MINODE *mip, int start)
{
  int i, blk;

  for(i = start; i < 12; i++){
    blk = mip->INODE.i_block[i + 1];
    mip->INODE.i_block[i + 1] = 0;
    mip->INODE.i_block[i] = blk;
  }
}

int shifting(char *buf, char *last, char *cp, int crl)
{
  printf("In shifting\n");
  printf("last = %s\n", ((DIR *)last)->name);
  printf("cp/dp = %s\n", dp->name);

  char temp[BLKSIZE];
  char *p;


  while(cp < buf + BLKSIZE){
    memcpy(temp, cp, dp->rec_len);
    printf("copy's name = %s\n", ((DIR *)temp)->name);
    memset(cp, 0, dp->rec_len);
    memcpy(last, temp, ((DIR *)temp)->rec_len);
    printf("last's name = %s\n", ((DIR *)last)->name);

    cp += ((DIR *)temp)->rec_len;
    dp = (DIR *)cp;
    p = last;
    last += ((DIR *)last)->rec_len;
  }

  printf("REC LEN BEFORE = %d\n", ((DIR *)p)->rec_len);
  ((DIR *)p)->rec_len += crl;
  printf("REC LEN After = %d\n", ((DIR *)p)->rec_len);
}

int rm_child(MINODE *pip, char *name)
{
  int i;
  char buf[BLKSIZE], *cp, *endcp, *last, temp[128];

  for(i = 0; i < 12; i++){
    if(pip->INODE.i_block[i] == 0) break;

    get_block(pip->dev, pip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    // get last dir_entry
    while(cp + dp->rec_len < buf + BLKSIZE){
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }

    endcp = cp;
    dp = (DIR *)buf;
    cp = buf;
    
    while(cp < buf + BLKSIZE){
      strcpy(temp, dp->name);
      temp[dp->name_len] = 0;

      printf("dp->name = %s\n", dp->name);
      printf("(temp) dp->name = %s\n", temp);
      if(!strcmp(temp, name)){
        printf("found %s\n", temp);
        // if first and only dir_entry
        if(dp->rec_len == BLKSIZE){
          printf("FIRST AND ONLY CASE\n");
          
          bdalloc(pip->dev, pip->INODE.i_block[i]);
          pip->INODE.i_block[i] = 0;
          pip->INODE.i_size -= BLKSIZE;

          if(pip->INODE.i_block[i + 1])
            compactDataBlocks(pip, i);
          break;
        }
        // if dir_entry at end
        else if(cp == endcp){
          printf("END CASE\nDeleting curent cp\n");
          int end_rec_len = dp->rec_len;
          memset(endcp, 0, end_rec_len);
          printf("end_rec_len = %d\n", end_rec_len);
          dp = (DIR *)last;
          printf("second to last dir = %s\n", dp->name);
          dp->rec_len += end_rec_len;
          printf("new rec_len = %d\n", dp->rec_len);
          cp = last;
          printf("Deleted end dir\n");
          break;
        }
        // shifting needed
        else{
          printf("SHIFT CASE\n");
          last = cp;
          cp += dp->rec_len;
          dp = (DIR *)cp;

          printf("last = %s\n", ((DIR *)last)->name);
          printf("cp = %s\n", dp->name);

          int crl = ((DIR *)last)->rec_len; // rec len of deleted dir_entry

          printf("deleting current dir_entry\n");
          memset(last, 0, ((DIR *)last)->rec_len);
          printf("deleted dir_entry. rec_len should = 0. rec_len = %d\n", ((DIR *)last)->rec_len);
          shifting(buf, last, cp, crl);
          break;
        }
      }
      last = cp;
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
    put_block(pip->dev, pip->INODE.i_block[i], buf);
  }
}

int rm_dir()
{
  int ino = getino(pathname), pino;
  MINODE *mip = iget(dev, ino), *pip;
  char name[128];

  // verify if dir and not busy
  if(!S_ISDIR(mip->INODE.i_mode)){printf("%s Is Not A DIR\n", pathname); iput(mip); return;}
  printf("REFCOUNT = %d\n", mip->refCount);
  if(mip->refCount > 1){printf("%s Is Busy\n", pathname); iput(mip); return;}

  // verify empty
  if(!verifyEmpty(mip)){printf("%s Is Not Empty\n", pathname); iput(mip); return;}

  get_myino(mip, &pino);
  pip = iget(dev, pino);

  get_myname(pip, ino, name);

  rm_child(pip, name);

  pip->INODE.i_links_count--;
  pip->dirty = 1;
  iput(pip);

  bdalloc(mip->dev, mip->INODE.i_block[0]);
  idalloc(mip->dev, mip->ino);
  iput(mip);
}

int link_file()
{
  char parent[64];
  char child[64];
  char temp[64];

  int oino = getino(pathname);
  MINODE *omip = iget(dev, oino);
  
  if(S_ISDIR(omip->INODE.i_mode)){printf("Cant link to DIR\n"); iput(omip); return;}
  if(getino(pathname2)){printf("File already exists\n"); iput(omip); return;}

  strcpy(temp, pathname2);
  strcpy(parent, dirname(temp));
  parent[strlen(parent)] = 0;
  printf("parent = %s\n", parent);

  strcpy(temp, pathname2);
  strcpy(child, basename(temp));
  child[strlen(child)] = 0;
  printf("child = %s\n", child);

  int pino = getino(parent);
  MINODE *pip = iget(dev, pino);

  enter_name(pip, oino, child);

  omip->INODE.i_links_count++;
  omip->dirty = 1;
  pip->dirty = 1;

  iput(omip);
  iput(pip);
}

int unlink_file()
{
  char parent[64];
  char child[64];
  char temp[64];
  char buf[BLKSIZE];
  int i;

  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);

  if(S_ISDIR(mip->INODE.i_mode)){printf("Cant unlink a DIR\n"); iput(mip); return;}
  if(mip->refCount > 1){printf("File is busy\n"); iput(mip); return;}

  strcpy(temp, pathname);
  strcpy(parent, dirname(temp));
  parent[strlen(parent)] = 0;
  printf("parent = %s\n", parent);

  strcpy(temp, pathname);
  strcpy(child, basename(temp));
  child[strlen(child)] = 0;
  printf("child = %s\n", child);

  int pino = getino(parent);
  MINODE *pip = iget(dev, pino);

  rm_child(pip, child);

  pip->dirty = 1;
  iput(pip);

  mip->INODE.i_links_count--;

  if(mip->INODE.i_links_count > 0)
    mip->dirty = 1;
  else{
    for(i = 0; i < 12; i++){
      if(mip->INODE.i_block[i] != 0){
        get_block(dev, mip->INODE.i_block[i], buf);
        memset(buf, 0, BLKSIZE);
        put_block(dev, mip->INODE.i_block[i], buf);
        bdalloc(dev, mip->INODE.i_block[i]);
        mip->INODE.i_block[i] = 0;
      }
    }
    idalloc(dev, ino);
  }

  iput(mip);
}

int symlink_file()
{
  char buf[BLKSIZE], old_name[64];

  int oino = getino(pathname);
  MINODE *omip = iget(dev, oino);

  if(!oino){printf("%s doesnt exist\n", pathname); iput(omip); return;}

  strcpy(old_name, pathname);
  strcpy(pathname, pathname2);
  int ino = creat_file();

  if(!ino){printf("Cant create link\n"); iput(omip); return;}
  //TODO: chmod to change file type to link

  MINODE *mip = iget(dev, ino);
  mip->INODE.i_mode = 0xA000;
  mip->INODE.i_mode |= S_IRWXU;
  mip->INODE.i_mode |= S_IRWXG;
  mip->INODE.i_mode |= S_IRWXO;
  strcpy(mip->INODE.i_block, old_name);
  mip->INODE.i_size = strlen(old_name);

  mip->dirty = 1;

  iput(mip);
  iput(omip);
}

int myreadlink(MINODE *mip, char *name)
{
  char test[64];
  strcpy(name, (char *)mip->INODE.i_block);
  name[strlen(name)] = 0;
  return strlen(name);
}

int mychmod()
{
  int ino = getino(pathname);
  MINODE *mip = iget(dev,ino);

  long mode = strtol(pathname2, NULL, 8);
  printf("octal mode %o\n", mode);
  mip->INODE.i_mode |= mode;

  mip->dirty = 1;
  iput(mip);
}

int utime()
{
  int ino = getino(pathname);
  MINODE *mip = iget(dev,ino);

  mip->INODE.i_atime = time(0L);

  mip->dirty = 1;
  iput(mip);
}

int truncate(MINODE *mip)
{
  printf("In truncate()\n");

  char buf[BLKSIZE], subbuf[BLKSIZE];
  int i, j;
  for(i = 0; i < 12; i++){
    if(mip->INODE.i_block[i] != 0){
      bdalloc(dev, mip->INODE.i_block[i]);
      mip->INODE.i_block[i] = 0;
    }
  }
  if(mip->INODE.i_block[12] == 0){return;}
  get_block(dev, mip->INODE.i_block[12], buf);
  for(i = 0; i < 256; i++){
    if(buf[i] != 0){
      bdalloc(dev, buf[i]);
      buf[i] = 0;
    }
  }
  put_block(dev, mip->INODE.i_block[12], buf);
  mip->INODE.i_block[12] = 0;

  if(mip->INODE.i_block[13] == 0){return;}
  get_block(dev, mip->INODE.i_block[13], buf);
  for(i = 0; i < 256; i++){
    for(j = 0; j < 256; j++)
      if(buf[i] != 0){
        get_block(dev, buf[i], subbuf);
        bdalloc(dev, subbuf[i]);
        subbuf[i] = 0;
    }
    put_block(dev, buf[i], subbuf);
    bdalloc(dev, buf[i]);
    buf[i] = 0;
  }
  put_block(dev, mip->INODE.i_block[13], buf);
  mip->INODE.i_block[13] = 0;

  mip->INODE.i_atime = time(0L);
  mip->INODE.i_mtime = time(0L);
  mip->INODE.i_size = 0;

  mip->dirty = 1;
  iput(mip);
}

int myopen()
{
  OFT *oftp;
  int ino = getino(pathname), i, mode, fd, offset;
  
  if(!ino)
    ino = creat_file();
  
  MINODE *mip = iget(dev, ino);

  mode = atoi(pathname2);
  printf("pathname2 = %s\n", pathname2);
  printf("mode = %d\n", mode);
  getchar();

  if(!ino){printf("File doesnt exist"); iput(mip); return -1;}
  if(!S_ISREG(mip->INODE.i_mode)){printf("Not a file\n"); iput(mip); return -1;}

  //TODO: Check permissions
  switch(mode){
    case 0:  offset = 0;
             break;
    case 1:  truncate(mip);
             offset = 0;
             break;
    case 2:  offset = 0;
             break;
    case 3:  offset = mip->INODE.i_size;
             break;
    default: printf("invalid mode\n"); iput(mip);
             return -1;
  }


  for(i = 0; i < NFD; i++){
    if(running->fd[i] == 0){fd = i; break;}
    if(i == NFD - 1){printf("Out of file descriptors\n"); iput(mip); return -1;}
  }

  for(i = 0; i < NOFT; i++){
    oftp = &openFileTable[i];
    if (oftp->inodeptr == mip && oftp->mode != 0){printf("File open with incompatible mode\n"); iput(mip); return -1;}
    if(oftp->refCount == 0){
      oftp->mode = mode;
      oftp->inodeptr = mip;
      oftp->offset = offset;
      oftp->refCount++;

      running->fd[fd] = oftp;
    }
  }

  if(mode == 0){mip->INODE.i_atime = time(0L);}
  else{mip->INODE.i_atime = time(0L); mip->INODE.i_mtime = time(0L);}

  mip->dirty = 1;
  return fd;
}

int open_file(){
  printf("opened fd = %d\n", myopen());
}

int myclose(int fd)
{
  if(fd > NFD || fd < 0){printf("fd out of range\n"); return -1;}
  if(!running->fd[fd]){printf("No OFT entry at fd %d\n", fd); return -1;}

  OFT *oftp = running->fd[fd];
  running->fd[fd] = 0;
  oftp->refCount--;

  if(oftp->refCount > 0){return 0;}
  MINODE *mip = oftp->inodeptr;

  iput(mip);
  return 0;
}

int close_file()
{
  int fd;
  fd = atoi(pathname);
  printf("fd = %d\n\n", fd);
  getchar();

  return (myclose(fd));
}

int mylseek(int fd, int position)
{
  if(fd > NFD || fd < 0){printf("fd out of range\n"); return -1;}
  if(!running->fd[fd]){printf("No OFT entry at fd %d\n", fd); return -1;}

  OFT *oftp = running->fd[fd];

  if(position > oftp->inodeptr->INODE.i_size || position < 0){printf("Position out of range\n"); return -1;}
  int original = oftp->offset;
  oftp->offset = position;

  return original;
}

int lseek_file()
{
  int fd, pos;
  fd = atoi(pathname);
  pos = atoi(pathname2);
  printf("fd = %d\nn = %d\n", fd, pos);
  getchar();

  return (mylseek(fd, pos));
}

int pfd()
{
  int i;
  printf("fd\tmode\t offset\t  INODE\n");
  printf("--------------------------------------\n");
  for(i = 0; i < NFD; i++){
    if(running->fd[i] != 0){
      printf("%d\t", i);

      if(running->fd[i]->mode == 0)
        printf("READ\t");
      else if(running->fd[i]->mode == 1)
        printf("WRITE\t");
      else if(running->fd[i]->mode == 2)
        printf("RW\t");
      else
        printf("APND\t");
      
      printf("%d\t",running->fd[i]->offset);
      printf("[%d, %d]\n", running->fd[i]->inodeptr->dev, running->fd[i]->inodeptr->ino);
    }
  }
}

int myread(int fd, char *buf, int nb)
{
  char *cq = buf, indirect[BLKSIZE];
  int blk, lbk, start, count = 0, offset = running->fd[fd]->offset, 
      avil = running->fd[fd]->inodeptr->INODE.i_size - offset;
  
  while(nb && avil){
    lbk = offset / BLKSIZE;
    start = offset % BLKSIZE;

    if(lbk < 12){
      blk = running->fd[fd]->inodeptr->INODE.i_block[lbk];
    }
    else if(lbk >= 12 && lbk < 256 + 12){
      blk = running->fd[fd]->inodeptr->INODE.i_block[12];
      get_block(dev, blk, indirect);

      blk = indirect[lbk - 12];
    }
    else{
      blk = running->fd[fd]->inodeptr->INODE.i_block[13];
      get_block(dev, blk, indirect);
      blk = indirect[(lbk - 268) / 256];
      get_block(dev, blk, indirect);

      blk = indirect[(lbk - 268) % 256];
    }

    get_block(dev, blk, indirect);
    char *cp = indirect + start;
    int remain = BLKSIZE - start;

    while(remain > 0){
      *cq++ = *cp++;
      offset++;
      count++;
      avil--; nb--; remain--;

      if(nb <= 0 || avil <= 0) break;
    }
  }
  running->fd[fd]->offset = offset;

  printf("bytes read = %d\n", count);
  printf("data read = %s\n", buf);

  return count;
}

int read_file()
{
  int fd, nb;
  fd = atoi(pathname);
  nb = atoi(pathname2);
  printf("fd = %d\nn = %d\n", fd, nb);
  getchar();

  if(fd > NFD || fd < 0){printf("fd out of range\n"); return -1;}
  if(!running->fd[fd]){printf("No OFT entry at fd %d\n", fd); return -1;}

  OFT *oftp = running->fd[fd];

  if(oftp->mode != 0 && oftp->mode != 3){printf("File not opened for read\n"); return -1;}

  return (myread(fd, readbuf, nb));
}

int write_file()
{

}


int test()
{

}