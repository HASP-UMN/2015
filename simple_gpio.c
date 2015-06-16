#include "simple_gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

int gpio_get_value(unsigned int gpio)
{
    int fd, value;
    char buf[MAX_BUF];
    char ch;
    
    snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
    
    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        perror("gpio/get-value");
        return fd;
    }
    
    read(fd, &ch, 1);
    
    if (ch != '0') {
        value = 1;
    } else {
        value = 0;
    }
    
    close(fd);
    return value;
}

int gpio_set_value(unsigned int gpio, unsigned int value)
{
    int fd, len;
    char buf[64];
    
    len = snprintf(buf,
                   sizeof(buf),"/sys/class/gpio/gpio%d/value", gpio);
    
    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/set-value");
        return fd;
    }
    
    if (value)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);
    
    close(fd);
    return 0;
}