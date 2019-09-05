#include <stdio.h>

typedef unsigned int u32;

char *ctable = "0123456789ABCDEF";
int  BASE = 10; 

int rpu(u32 x)
{  
    char c;
    if (x){
       c = ctable[x % BASE];
       rpu(x / BASE);
       putchar(c);
    }
}

int printu(u32 x)
{
    BASE = 10;
   (x==0)? putchar('0') : rpu(x);
   putchar(' ');
}
int printx(u32 x)
{
    BASE = 16;
    putchar('0');
    putchar('x');
   (x==0)? putchar('0') : rpu(x);
   putchar(' ');
}
int printo(u32 x)
{
    BASE = 8;
    putchar('0');
   (x==0)? putchar('0') : rpu(x);
   putchar(' ');
}
int printd(int x)
{
    if(x < 0){
        x = -x;
        putchar('-');
    }

    BASE = 10;
   (x==0)? putchar('0') : rpu(x);
   putchar(' ');
}

void prints(char *s){
    while(*s != 0) putchar(*s++);
    
    putchar('\n');
}

void myprintf(char *fmt, ...){
    char *cp = fmt;
    int *FP = (int *) getebp();
    int *ip = FP + 3;

    while(*cp != 0){
        if(*cp == '%'){
            cp++;
            switch(*(cp)){
                case 'c': putchar(*ip);
                    break;
                case 's': prints((char *) *ip);
                    break;
                case 'u': printu(*ip);
                    break;
                case 'd': printd(*ip);
                    break;
                case 'o': printo(*ip);
                    break;
                case 'x': printx(*ip);
                    break;
                default:
                    printf("NOPE\n");
            }
            ip++;
        }
        else if(*cp == '\n'){
            putchar('\n');
            putchar('\r');
        }
        else{
            putchar(*cp);
        }
        cp++;
    }
}

// int main(void){


//     myprintf("kadir %c %s \n%u %d %o %x\n", 's', "nour", 12, -12, 12, 12, 0, 0, 0, 0, 0);

//     return 0;
// }