/*********** util.c file ****************/

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern OFT openFileTable[NOFT];

extern char   gpath[256];
extern char   *name[64];
extern int    n;

extern int    gfd, dev;
extern int    nblocks, ninodes, bmap, imap, inode_start;
extern char   line[256], cmd[32], pathname[64], pathname2[64];

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8; j = bit % 8;
  if(buf[i] & (1 << j))
    return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8; j = bit % 8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8; j = bit % 8;
  buf[i] &= ~(1 << j);
}

int tokenize(char *pathname)
{
  char *s;
  strcpy(line, pathname);
  n = 0;

  s = strtok(line, "/");
  while(s){
    name[n++] = s;
    s = strtok(NULL, "/");
  }
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, disp;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];

    if (mip->refCount && mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
      //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk  = (ino-1) / 8 + inode_start;
       disp = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + disp;
       // copy INODE to mp->INODE
       mip->INODE = *ip;

       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
  return 0;
}

int iput(MINODE *mip)
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE *ip;

 if (mip==0) 
     return;

 mip->refCount--;
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 
 /* write back */
 printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino); 

 block =  ((mip->ino - 1) / 8) + inode_start;
 offset =  (mip->ino - 1) % 8;

 /* first get the block containing this inode */
 get_block(mip->dev, block, buf);

 ip = (INODE *)buf + offset;
 *ip = mip->INODE;

 put_block(mip->dev, block, buf);

}

int search(MINODE *mip, char *name)
{
  int i;
  char *cp, temp[256], sbuf[BLKSIZE];

  /******************testing*****************/
  // printf("block num of root %d\n", mip->INODE.i_block[0]);
  // getchar();
  /*****************************************/

  for(i=0; i<12; i++){
    if(mip->INODE.i_block[i] == 0)
      return 0;
    get_block(mip->dev, mip->INODE.i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;

    while(cp < sbuf + BLKSIZE){
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;

      printf("%8d%8d%8u %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
      if(!strcmp(temp, name)){
        printf("found %s : ino = %d\n", name, dp->inode);
        return dp->inode;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return 0;
}

int getino(char *pathname)
{
  int i, ino, blk, disp;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;

  if (pathname[0]=='/')
    mip = iget(dev, 2);
  else
    mip = iget(running->cwd->dev, running->cwd->ino);

  tokenize(pathname);

  for (i=0; i<n; i++){
      printf("===========================================\n");
      ino = search(mip, name[i]);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }
      iput(mip);
      mip = iget(dev, ino);

      if(mip->mounted){
        dev = mip->mountptr->dev;

        iput(mip);
        mip = iget(dev, 2);

        printf("Crossed Mount Point\n");
      }
   }
   iput(mip);
   return ino;
}
int get_myino(MINODE *mip, int *parent_ino)
{
  *parent_ino = search(mip, "..");
  return search(mip, ".");
}

int get_myname(MINODE *parent_minode, int my_ino, char *my_name)
{
  char ibuf[BLKSIZE], *cp;
  get_block(parent_minode->dev, parent_minode->INODE.i_block[0], ibuf);
  dp = (DIR *)ibuf;
  cp = ibuf;

  while(cp < ibuf + BLKSIZE){
    if(dp->inode == my_ino){
      strncpy(my_name, dp->name, dp->name_len);
      my_name[dp->name_len] = 0;
      return 1;
    }
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  return 0;
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];
  // dec free inode count by 1 in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];
  // inc free inode count by 1 in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int decFreeBlocks(int dev)
{
  char buf[BLKSIZE];
  // dec free blocks count by 1 in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

int incFreeBlocks(int dev)
{
  char buf[BLKSIZE];
  // inc free blocks count by 1 in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}

int ialloc(int dev)
{
  int i;
  char buf[BLKSIZE];
  
  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       put_block(dev, imap, buf);
       decFreeInodes(dev);
       return i+1;
    }
  }
  return -1;
}

int idalloc(int dev, int ino)
{
  char buf[BLKSIZE];

  if(ino > ninodes){ printf("ino %d out of range\n", ino); return;}
  //read inode_bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);

  put_block(dev, imap, buf);
  incFreeInodes(dev);
}

int balloc(int dev)
{
  int i;
  char buf[BLKSIZE];

   // read inode_bitmap blocks
  get_block(dev, bmap, buf);
  int z = 0, w = 0;
  printf("------block bitmap------\n");
  for(i = 0; i < nblocks; i++){
    if(tst_bit(buf, i) == 0){
      printf("0");
      z++;
    }
    else{
      w++;
      printf("1");
    }

    if(i % 24 == 0 && i != 0){printf("\n");}
  }
  printf("\n0's = %d\n", z);
  printf("1's = %d\n", w);

  for (i=0; i < nblocks; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       put_block(dev, bmap, buf);
       decFreeBlocks(dev);
       return i;
    }
  }
  return -1;
}

int bdalloc(int dev, int blk)
{
  char buf[BLKSIZE];

  if(blk > nblocks){printf("blk %d out of range\n", blk); return;}

   //read inode_bitmap blocks
  get_block(dev, bmap, buf);
  int z = 0, w = 0 , i = 0;
  printf("------block bitmap------\n");
  for(i = 0; i < nblocks; i++){
    if(tst_bit(buf, i) == 0){
      printf("0");
      z++;
    }
    else{
      w++;
      printf("1");
    }

    if(i % 24 == 0 && i != 0){printf("\n");}
  }
  printf("\n0's = %d\n", z);
  printf("1's = %d\n", w);

  //read blk_bitmap block
  get_block(dev, bmap, buf);
  clr_bit(buf, blk);

  put_block(dev, bmap, buf);
  incFreeBlocks(dev);
}