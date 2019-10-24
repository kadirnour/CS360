#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct ext2_super_block   SUPER;
typedef struct ext2_group_desc    GD;
typedef struct ext2_inode         INODE;
typedef struct ext2_dir_entry_2   DIR;
#define BLKSIZE 1024


SUPER *sp;
GD *gp;
INODE *ip;
DIR *dp;
char *cp;
char buf[BLKSIZE], *names[128];
int fd, inode_start;


int get_block(int fd, int blk, void *buf)
{
    lseek(fd, blk * BLKSIZE, SEEK_SET);
    return read(fd, buf, BLKSIZE);
}
INODE * get_inode(int ino, char *buf)
{
    lseek(fd, inode_start * BLKSIZE + (ino - 1) * sizeof(INODE), SEEK_SET);
    read(fd, buf, sizeof(INODE));
    return (INODE *)buf;
}

INODE * root(int fd)
{
    int i;
    lseek(fd, inode_start * BLKSIZE + sizeof(INODE), SEEK_SET);
    read(fd, buf, BLKSIZE);

    ip = (INODE *)buf;

    lseek(fd, ip->i_block[0] * BLKSIZE, SEEK_SET);
    read(fd, buf, BLKSIZE);

    dp = (DIR *)buf;
    cp = buf;

    printf("inode\trec_len\tname_len\tname\n");
    while(cp < buf + BLKSIZE){
        printf("%d\t%d\t%d\t\t%s\n", dp->inode, dp->rec_len, dp->name_len, dp->name);

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    return ip;
}

int tokenize(char *pathname)
{
    int n = 0;
    char temp[256], *s;
    strcpy(temp, pathname);

    s = strtok(pathname, "/");

    while(s){
        names[n++] = s;
        s = strtok(NULL, "/");
    }

    return n;
}
INODE * path2inode(int fd, char *pathname)
{
    
}
int search(INODE *inode, char *name)
{
    get_block(fd, inode->i_block[0], buf);

    dp = (DIR *)buf;
    cp = buf;
    while(cp < buf + BLKSIZE){
        char temp[128];
        strcpy(temp, dp->name);
        temp[strlen(temp)] = 0;

        if(!strcmp(name, temp)){
            return dp->inode;
        }

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    return 0;
}
int do_indirect(int data_block)
{
    int i;
    int indirect[BLKSIZE];
    get_block(fd, data_block, indirect);

    for(i=0;i<256;i++){
        if(indirect[i])
            printf("%d ", indirect[i]);
    }
}
int do_double_indirect(int data_block)
{
    int i;
    int indirect[BLKSIZE];
    get_block(fd, data_block, indirect);

    for(i=0;i<256;i++){
        if(indirect[i])
            do_indirect(indirect[i]);
    }
}

int main(int argc, char *argv[])
{
    int i, ino;
    fd = open("diskimage", O_RDONLY);
    if(fd < 0) {printf("open device failed\n"); exit(1);}

    get_block(fd, 1, buf);
    sp = (SUPER *)buf;
    
    if(sp->s_magic != 0xEF53){printf("Not EXT2 FS\n"); exit(2);}

    get_block(fd, 2, buf);
    gp = (GD *)buf;

    inode_start = gp->bg_inode_table;

    printf("***********Root inode***********\n");
    ip = root(fd);
    printf("Press any key to continue: ");
    getchar();

    char pathname[256];
    strcpy(pathname, argv[1]);

    int n = tokenize(pathname);

    printf("n = %d\n", n);

    for(i=0;i<n;i++){
        printf("Searching for %s\n", names[i]);
        ino = search(ip, names[i]);
        if(!ino){
            printf("Cant find %s\n", names[i]);
            exit(1);
        }
        get_inode(ino, buf);
        ip = (INODE *)buf;
    }

    printf("ino = %d\n", ino);
    printf("size = %d\n", ip->i_size);

    for(i=0; i<15;i++){
        printf("i_block[%d] = %d\n", i, ip->i_block[i]);
    }

    if(ip->i_block[12]){
        printf("------------Do indirect block------------\n");
        do_indirect(ip->i_block[12]);
        printf("\n");
    }
    if(ip->i_block[13]){
        printf("\n------------Do double indirect block------------\n");
        do_double_indirect(ip->i_block[13]);
        printf("\n");
    }
    if(ip->i_block[14])
        printf("------------Do triple indirect block------------\n");
        //not needed this assignment

    printf("\n-----------------------------------------\n");

    return 0;
}