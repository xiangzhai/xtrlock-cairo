// Copyright (C) 2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) 
{
    int count = 10;
    while (count) {
        printf("Please wait %d second(s) to notify send\n", count--);
        sleep(1);
    }
    for (int i = 0; i < 13; i++) {
        system("notify-send 'Hello world!' 'This is an example notification.' "
               "--icon=dialog-information -t 10000");
        sleep(3);
    }
    return 0;
}
