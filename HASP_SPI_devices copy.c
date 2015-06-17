#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include "globaldefs.h"
#include "spi.h"
#include "HASP_SPI_devices.h"
#include "timing.h"

int init_IMU(struct imu* imu_ptr)
{    
    //int fd = (int)malloc(sizeof(int));
    int fd = spi_init(IMU_device,IMU_mode,IMU_bpw,IMU_speed);
    //fprintf(stderr,"fd = %d.\n",fd);
    
    imu_ptr->badDataCounter = 0;
    imu_ptr->rstTime = 0;
    
    imu_ptr->spi_handle = fd;
    
    return 0;
}


int read_IMU(struct imu *imu_ptr)
{
    
    if ((get_timestamp_ms() - imu_ptr->rstTime) < 250)
    {
        return -1;
    }
    
    //To read the IMU a full-duplex SPI transfer is used, which only sends and receives
    //  8-bits at a time; meaning, the size of one index in the tx and rx buffers is
    //  simply one uint8_t (one byte).
    uint8_t *tx = (uint8_t *)malloc(12*2*sizeof(uint8_t));
    uint8_t *rx = (uint8_t *)malloc(12*2*sizeof(uint8_t));
    
    //The values we are storing are either 10-bit or 12-bit integers, thus the OUTPUT data
    //  must contain variables of at least this size. Note sizeof(int) = 32 bits.
    int *outputData = (int *)malloc(12*sizeof(int));
    
    //Other variables needed for the bit-masking loop.
    int tmp, i;
    int responseLength = 12*2;
    
    //0x3E00 command begins burst mode output; request all 12 data registers
    //0x3E00 is a 16-bit command, so we use the FIRST TWO indicies of the tx buffer.
    //It does not matter what goes into the rx buffer for this step. We are not gathering
    //  any data, but the IMU driver is written with a full-duplex transfer for simplicity.
    tx[0] = 0x3E;
    tx[1] = 0x00;
    spi_transfer(imu_ptr->spi_handle, tx, rx, 2);
    
    //This pause is necessary to ensure that the sensor has time to respond before the read attempt.
    //The SPI bit rate is 500kHz, with a 15 usec delay in between each 16-bit frame, and a 1 usec
    //  delay between the chip select down and clock start/stop. Thus, a 26-byte
    //  read/write will take at least 600 usec. This has been measured to be approximately 630 usec.
    usleep(1000);
    
    //Here we fill the tx buffer with zeros. Probably not necessary, but it is to ensure
    //  that the 'burst mode' command isn't sent twice
    for(i=0;i<12*2;i++)
    {
        tx[i] = 0x00;
        rx[i] = 0x00;
    }
    
    //Read 12*2 = 24 bytes into the response buffer. Could have passed '24' as the last argument
    //  to spi_transfer(), but 12*2 is easier to understand and debug.
    spi_transfer(imu_ptr->spi_handle, tx, rx, 12*2);
    
    
    //Timestamp the results. NOTE: THIS IS A QUICK FIX AND SHOULD NOT BE USED IN FLIGHT
    //We have an external clock module in addition to the BeagleBone's clock, which should
    //  provide much more a precise approximation. All we need is a reference from the beginning
    //  of the flight. (int)time(NULL) returns an int of the time since the UNIX epoch in seconds.
    //  Whichever time function is used must be called in the initialization sequence (before the
    //  while(1){} loop) and all time measurements during flight are referenced from that point.
    //imu_ptr->time = get_timestamp_ms();;
    // generate timestamp
    struct timeval spec;
    
    gettimeofday(&spec, NULL);
    long stime = spec.tv_sec;
    long mtime = spec.tv_usec/1000;
    
    imu_ptr->stime = stime;
    imu_ptr->mtime = mtime;
    // end timestamp
    
    
    //Supply voltage and all inertial sensor readings are 14 bit. Temperature and ADC are
    //  12 bit. Unused bits are masked after first byte is read. This byte is then shifted up
    //  8 bits, and the lower byte is read and added to it to obtain the raw value (either 12
    //  or 14 bit, and either binary or two's complement depending on which register is being
    //  read). If the number is two's complement, it is converted to decimal.
    for(i=2; i<responseLength-4; i+=2)
    {
        tmp = ((rx[i] & 0x3F)<<8) + rx[i+1];
        
        if (tmp > 0x1FFF) {
            outputData[i/2] = -(0x4000-tmp);
        }
        else {
            outputData[i/2] = tmp;
        }
        
    }
    
    int tmp1 = rx[0];
    int tmp2 = rx[1];
    outputData[0] = ((tmp1 & 0x3F) << 8 ) + tmp2;
    
    tmp1 = rx[20];
    tmp2 = rx[21];
    tmp1 = ((tmp1 & 0x0F) << 8) + tmp2;
    if(tmp1 > 0x07FF)
    {
        tmp1 = -(0x1000 - tmp1);
    }
    
    outputData[10] = tmp1;
    tmp1 = rx[22];
    tmp2 = rx[23];
    outputData[11] = ((tmp1 & 0x0F) << 8) + tmp2;
    
    //memcpy(imu_ptr->response, outputData, 12*sizeof(int));
    
    //Data from the IMU is valid only if supply voltage is within 4.75 to 5.25.
    if((outputData[0]*0.002418 > 4.25) && (outputData[0]*0.002418 < 5.25))
    {
        //fprintf(stderr,"data_valid\n");
        imu_ptr->supply_raw = outputData[0];
        imu_ptr->gyroX_raw  = (int)outputData[1];
        imu_ptr->gyroY_raw  = (int)outputData[2];
        imu_ptr->gyroZ_raw  = (int)outputData[3];
        imu_ptr->accelX_raw = (int)outputData[4];
        imu_ptr->accelY_raw = (int)outputData[5];
        imu_ptr->accelZ_raw = (int)outputData[6];
        imu_ptr->magX_raw = (int)outputData[7];
        imu_ptr->magY_raw = (int)outputData[8];
        imu_ptr->magZ_raw = (int)outputData[9];
        imu_ptr->temp_raw  = outputData[10];
        imu_ptr->ADC_raw = outputData[11];
    }
    else if ((outputData[0]*0.002418 < 4.25) || (outputData[0]*0.002418 > 5.25))
    {
        imu_ptr->badDataCounter++;
        if (imu_ptr->badDataCounter > 2000)
        {
            gpio_set_value(IMU_RST, 0);
            usleep(10);
            gpio_set_value(IMU_RST, 1);
            
            imu_ptr->rstTime = get_timestamp_ms();
            imu_ptr->badDataCounter = 0;
        }
        if(tx)
            free(tx);
        if(rx)
            free(rx);
        if(outputData)
            free(outputData);
        
        return -1;
    }
    
    if(tx)
        free(tx);
    if(rx)
        free(rx);
    if(outputData)
        free(outputData);
    return 0;
}

int close_IMU(struct imu *imu_ptr)
{
    
    close(imu_ptr->spi_handle);
    return 0;
}