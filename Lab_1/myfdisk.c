#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct partition {
	u8 drive;             /* drive number FD=0, HD=0x80, etc. */

	u8  head;             /* starting head */
	u8  sector;           /* starting sector */
	u8  cylinder;         /* starting cylinder */

	u8  sys_type;         /* partition type: NTFS, LINUX, etc. */

	u8  end_head;         /* end head */
	u8  end_sector;       /* end sector */
	u8  end_cylinder;     /* end cylinder */

	u32 start_sector;     /* starting sector counting from 0 */
	u32 nr_sectors;       /* number of of sectors in partition */
}Partition;


void printp(Partition *p){
    myprintf("drive=%u head=%u sector=%u cylinder=%u sys_type=%u\nendh=%u ends=%u endc=%u start_sector=%d nr_sector=%d\n\n", p->drive, p->sector,
    p->cylinder, p->sys_type, p->end_head, p->end_sector, p->end_cylinder, 
    p->start_sector, p->nr_sectors);
}

int main(){
    
    int fd, i, carry;
    char buf[512];

    fd = open("vdisk", O_RDONLY);  // check fd value: -1 means open() failed
    read(fd, buf, 512);            // read sector 0 into buf[ ]

    Partition *p;
    p = (Partition *) (buf + 0x1BE);
    
    myprintf("Device\tStart\tEnd\tSectors\tType\n");
    i = 0;
    for(i; i < 4; i++){
        myprintf("P%d\t%d\t%d\t%d\t%x\n", i + 1, p->start_sector, (p->start_sector + p->nr_sectors) - 1, p->nr_sectors, p->sys_type);
        p++;
    }
    p--; //Beginning of P4


    p->start_sector = 0; //so the loop can run effectively

    do{
        carry = p->start_sector + 1440; //begin sector

        lseek(fd, (long) (p->start_sector + 1440) * 512, SEEK_SET); // goto start of P_
        read(fd, buf, 512);

        p = (Partition *) (buf + 0x1BE); //P_ local MBR

        myprintf("P%d\t%d\t%d\t%d\t%x\n", i + 1, (p->start_sector + carry), 
        ((p->start_sector + carry) + p->nr_sectors - 1), p->nr_sectors, 
        p->sys_type);

        i++; //count
        p++;
    }while(p->start_sector != 0);

    //0 start sector (1440 technically)

    //begin sector is start sector plus current p->start_sector

    //lseek to start sector

    //print shit

    //p++ gives me the start sector of the next partition

}