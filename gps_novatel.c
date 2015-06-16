#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

#include "globaldefs.h"
#include "simple_gpio.h"
#include "gps_novatel.h"
#include "timing.c"

#define CRC32_POLYNOMIAL   0xEDB88320L
#define GPS_MAX_MSG_SIZE 300
#define PV_PIN 44
#define header_length 28
#define response_length 512

unsigned char response[512];
const char asterisk[] = "*";

void init_GPS(struct gps* gpsData_ptr)
{
    
    //uart5 configuration using termios
    struct termios uart5;
    int fd;
    
    //open uart5 for tx/rx, not controlling device
    if((fd = open("/dev/ttyO5", O_RDWR | O_NOCTTY)) < 0) {
        fprintf(stderr,"Unable to open UART5 access in gps_novatel.c\n");
    }
    //get attributes of uart5
    if(tcgetattr(fd, &uart5) < 0) {
        fprintf(stderr,"Could not get attributes of UART5. Error in gps_novatel.c\n");
    }
    //set Baud rate
    if (cfsetospeed(&uart5, B115200) < 0) {
        fprintf(stderr,"Could not set baud rate\n. gps_novatel.c");
    }
    else {
        //fprintf(stderr,"\n\nBaud rate: 115200\n");
    }
    
    //set attributes of uart5
    uart5.c_iflag = 0;
    uart5.c_oflag = 0;
    uart5.c_lflag = 0;
    tcsetattr(fd, TCSANOW, &uart5);
    gpsData_ptr->port = fd;
    gpsData_ptr->badDataCounter = 0;
    gpsData_ptr->posValFlag = 0;
    gpsData_ptr->lastPosVal = 0;
    return;
}


void endian_swap(uint8_t *buf, int index, int count)
{
    int i;
    uint8_t tmp;
    
    for (i=0;i<(count/2);i++) {
        tmp = buf[index+i];
        buf[index+i] = buf[index+count-i-1];
        buf[index+count-i-1] = tmp;
    }
    return;
}

unsigned int CRC32Value(int i)
{
    int j;
    unsigned int ulCRC;
    ulCRC = i;
    for (j=8;j>0;j--)
    {
        if (ulCRC & 1)
            ulCRC = (ulCRC >> 1)^CRC32_POLYNOMIAL;
        else
            ulCRC >>= 1;
    }
    
    return ulCRC;
}


unsigned int CalculateBlockCRC32(unsigned int ulCount, unsigned char *ucBuffer)
{
    unsigned int ulTemp1;
    unsigned int ulTemp2;
    unsigned int ulCRC = 0;
    
    while (ulCount-- != 0)
    {
        ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
        ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
        ulCRC = ulTemp1 ^ ulTemp2;
    }
    
    return ulCRC ;
}


int read_GPS(struct gps *gpsData_ptr)
{
    unsigned int state = 0;
    unsigned int counter = 0;
    
    int i = 0;
    int pCount = 0;
    int bytesInLocalBuffer;
    
    unsigned long CRC_computed;
    unsigned long CRC_read;
    unsigned int CRC_readstr[4];
    
    bytesInLocalBuffer = read(gpsData_ptr->port, response, 512);
    
    //for(i=0;i<144;i++) {
    //fprintf(stderr,"%x, ",response[i]);}
    
    if(bytesInLocalBuffer<144)
    {
        // Exit read_gps function in case entire message isn't read
        fprintf(stderr, "gps port returned less than 144 bytes: exiting read_gps\n");
        return -1;
    }
    
    while (counter < bytesInLocalBuffer){
        switch(state) {
            case 0:
                if(response[counter]==0xAA){
                    //fprintf(stderr,"Read 0xAA\n");
                    state++;
                    counter++;
                }
                else
                {
                    state = 0;
                    counter++;
                }
                break;
            case 1:
                if(response[counter]==0x44){
                    //fprintf(stderr,"Read 0x44\n");
                    state++;
                    counter++;
                }
                else
                {
                    state = 0;
                    counter++;
                }
                break;
            case 2:
                if(response[counter]==0x12){
                    //fprintf(stderr,"Read 0x12\n");
                    state++;
                    //counter++;
                }
                else
                {
                    state=0;
                    counter++;
                }
                break;
            case 3:
                
                //memcpy(gpsData_ptr->responseBuffer,&response[counter-2],144);
                
                //	endian_swap(response,pCount,2);
                //	for(i=0;i<bytesInLocalBuffer;i++)
                //		printf("%d:%x ",i,response[i]);
                //	printf("Response length: %d\n", bytesInLocalBuffer);
                
                // Calculate CRC
                
                //fprintf(stderr,"%0X %0X %0X\n",*(response+counter-2),*(response+counter-2+1),*(response+counter-2+2));
                CRC_read = *((unsigned long *)(&response[140]));
                response[139] = '\0';
                CRC_computed = CalculateBlockCRC32(140,response+counter-2);
                //fprintf(stderr,"CRC Calculated = %x, CRC read = %x\n",CRC_computed,CRC_read);
                if (CRC_computed != CRC_read) {
                    state = 0;
                    counter = 0;
                    //fprintf(stderr,">>CRC Err!\n");
                    gpsData_ptr->badDataCounter++;
                    
                    if(gpsData_ptr->badDataCounter > 5)
                    {
                        // reset the GPS!
                        //gpio_set_value(GPS_RST, 0);
                        //gpio_set_value(GPS_RST, 1);
                        
                        gpsData_ptr->badDataCounter = 0;
                    }
                    
                    return -1;
                }
                
                memcpy(gpsData_ptr->responseBuffer,&response[counter-2],144);//copy data read from gps port to gps struct
                
                if( !gpio_get_value(GPS_POS_VAL) && ((get_timestamp_ms() - gpsData_ptr->lastPosVal)>300000) && (gpsData_ptr->posValFlag == 1))
                {
                    // reset the GPS!
                    //		printf("GPS FLAG1\n");
                    //gpio_set_value(GPS_RST, 0);
                    //gpio_set_value(GPS_RST, 1);
                    gpsData_ptr->posValFlag = 0;
                    return -1;
                }
                else if(gpio_get_value(GPS_POS_VAL))
                {
                    // set flag and time
                    gpsData_ptr->posValFlag = 1;
                    gpsData_ptr->lastPosVal = get_timestamp_ms();
                }
                
                pCount = counter - 2;
                
                //endian_swap(response,pCount+14,2);
                //endian_swap(response,pCount+16,4);
                //endian_swap(response,pCount+28+8,8);
                //endian_swap(response,pCount+28+16,8);
                //endian_swap(response,pCount+28+24,8);
                
                //gpsData_ptr->timeStatus = *((uint8_t *)(&response[13]));
                //printf("GPS Time Status: %d\n",gpsData_ptr->timeStatus);
                //gpsData_ptr->weekRef = *((uint16_t *)(&response[14]));
                gpsData_ptr->timeStatus = *((uint8_t *)(response + 13));
                gpsData_ptr->weekRef = *((uint16_t *)(response + 14));
                gpsData_ptr->time = *((long *)(response + 16));
                
                // Set system time from GPS if it is good
                if(gpsData_ptr->timeStatus == 80 || gpsData_ptr->timeStatus == 100 || gpsData_ptr->timeStatus == 120 ||gpsData_ptr->timeStatus == 140 || gpsData_ptr->timeStatus == 160 || gpsData_ptr->timeStatus == 170 || gpsData_ptr->timeStatus == 180)
                    //time statuses explained on page 27 of oemstar firmware manual
                {
                    time_t unixSeconds = 315964800 + (24*7*3600*gpsData_ptr->weekRef + gpsData_ptr->time/1000);
                    // first number in above line is seconds between 1/1/70 and 1/6/80, or something like that
                    // gpsData_ptr->weekRef is the number of weeks since 1/6/80
                    // gpsData_ptr->time is "milliseconds into gps reference week" so time/1000 gives sec into gps ref week
                    
                    //time_t * timeSeconds = &unixSeconds;
                    //stime(timeSeconds);
                    stime(&unixSeconds);
                }		
                
                //gpsData_ptr->Xe = *((double *)(&response[36]));
                //gpsData_ptr->Ye = *((double *)(&response[44]));
                //gpsData_ptr->Ze = *((double *)(&response[52]));
                gpsData_ptr->Xe = *((double *)(response + 36));
                gpsData_ptr->Ye = *((double *)(response + 44));
                gpsData_ptr->Ze = *((double *)(response + 52));
                state = 0;
                counter = bytesInLocalBuffer; // breaks while()
//                for(i = 0; i < 512; i++)
//                {
//                    response[i]='\0';
//                }
                bzero(response, response_length);
                break;
        }
    }
    
    return 0;
}