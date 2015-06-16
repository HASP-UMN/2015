#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "spi.h"
#include "globaldefs.h"

inline int file_valid(const char *path)
{
    if (access(path, F_OK) == 0)
    {
        return 1;
    }
    return 0;
}


int spi_init(const char *device, uint8_t mode, uint8_t bpw, uint32_t speed)
{
    
    int status = 0;
    if(file_valid(device)){
        //fprintf(stderr,"file %s is valid.\n",device);
    }
    else
    {
        fprintf(stderr,"file %s is not valid\n",device);
    }
    
    int fd = open(device, O_RDWR);
    if(fd < 0)
    {
        fprintf(stderr,"error opening SPI device\n");
    }
    else
    {
        //fprintf(stderr,"SPI device @ %s successfully initialized.\n",device);
    }
    
    status = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if(status == -1)
    {
        fprintf(stderr,"error setting mode\n");
    }
    else
    {
        //fprintf(stderr,"Mode set to %d.\n",mode);
    }
    
    
    status = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bpw);
    if(status == -1)
    {
        fprintf(stderr,"error setting bits per word\n");
    }
    else
    {
        //fprintf(stderr,"Bits per word set to %d.\n",bpw);
    }
    
    
    status = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if(status == -1)
    {
        fprintf(stderr,"error setting max speed\n");
    }
    else
    {
        //fprintf(stderr,"Speed set to %f MHz.\n",speed*.000001);
    }
    
    return fd;
    
}


int spi_transfer(int spi_handle, uint8_t *tx, uint8_t *rx, uint32_t len)
{
    
    //fprintf(stderr,"performing duplex rw transfer of %d bytes\n", len);
    
    if(spi_handle < 0 || rx == NULL || tx == NULL)
    {
        fprintf(stderr,"spi | rx | tx was NULL\n");
        return EXIT_FAILURE;
    }
    
    if(len <= 0)
    {
        fprintf(stderr,"length was less than zero\n");
        return EXIT_FAILURE;
    }
    
    int ret;
    
    struct spi_ioc_transfer tr =
    {
        .rx_buf = (unsigned long) rx,
        .tx_buf = (unsigned long) tx,
        .len = len,
        .cs_change = 0,
    };
    
    ret = ioctl(spi_handle, SPI_IOC_MESSAGE(1), &tr);
    
    if(ret < 1)
    {
        //fprintf(stderr,"failed duplex transfer\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
    
}