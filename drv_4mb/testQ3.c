#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

int lcd;
//int SCULL_HELLO=0;
//int SCULL_SETMSG=1;
//int SCULL_GETMSG=2;
#define SCULL_IOC_MAGIC 'k'
#define SCULL_HELLO _IO(SCULL_IOC_MAGIC, 1)
#define SCULL_SETMSG _IOW(SCULL_IOC_MAGIC,  2, int)
#define SCULL_GETMSG _IOR(SCULL_IOC_MAGIC,  3, int)
#define SCULL_SET_GETMSG _IOWR(SCULL_IOC_MAGIC,  4, int)
char *usr_msg;

void test(){
    int k, i, sum;
    char s[3];

    memset(s, '2', sizeof(s));
    printf("Test begin!\n");

    k=write(lcd, s, sizeof(s));
    printf("written=%d\n", k);

    k=ioctl(lcd, SCULL_HELLO);
    printf("result=%d\n", k);
    
    //define the message to be written to device
    char message[] = "A01485528";
    printf("pointer of message: %d \n", message);
    printf("message from dev_msg: %s \n", message);
    k=ioctl(lcd, SCULL_SETMSG, message);
    //printf("Write to dev_msg: %d\n", k);

    k=ioctl(lcd, SCULL_GETMSG, usr_msg);
    //printf("pointer of message: %d \n", usr_msg);
    //printf("usr_msg: %s\n", usr_msg);

    k=ioctl(lcd, SCULL_SET_GETMSG, usr_msg);
    printf("usr_msg written: %s\n", message);
}

int main(int argc, char **argv){
    lcd=open("/dev/scull", O_RDWR);
    if(lcd == -1){
        perror("unable to open lcd");
        exit(EXIT_FAILURE);
    }
    test();
    close(lcd);
    return 0;
}
