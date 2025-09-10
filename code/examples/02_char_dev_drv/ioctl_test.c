/*
    File:           ioctl_test.c
    Description:    This is an easy process for testing char_dev_drv module
    Author:         Cai
    Version:        v1.0    10Step25
*/

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    // In old Version:  first need run "mknod /dev/char_dev c 202 0" to create device file
    int my_dev = open("/dev/char_dev", 0);
    if(my_dev < 0)
    {
        perror("open file");
    }
    else
    {
        ioctl(my_dev, 100, 110);
        close(my_dev);
    }

    return 0;
}