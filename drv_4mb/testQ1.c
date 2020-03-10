#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    FILE *fp;
    int length = 5000000;
    char chr = 'a';
    char outStr[length];
    int i;
    int num, size_device;
    struct stat st;

    for (i=1; i<length-1; i++){
	outStr[i] = chr;
    }
    outStr[0] = 'X';
    outStr[length-1] = 'Z';
    //printf("The size of outStr is %d", sizeof(outStr));	

    fp = fopen("/dev/a4", "w");
    num = fputs(outStr, fp);
    fclose(fp);
    
    printf("The return value of the fputs is %i\n", num);
    
    return 0;
}
