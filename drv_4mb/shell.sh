#!/bin/sh
dmesg -c
rmmod 4Mbyte
make
insmod 4Mbyte.ko

gcc testQ3.c -o test
./test
dmesg | tail
