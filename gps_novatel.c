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
#define header_length 28
#define response_length 512

unsigned char response[512];
const char asterisk[] = "*";

void init_GPS(struct gps* gpsData_ptr) {
    
    // GPS RECEIVER UART PORT CONFIG USING TERMIOS
    struct termios GPS_UART;
    int fd;
    
    // OPEN SERIAL FOR TX/RX, NOT CONTROLLING DEVICE
    if((fd = open("/dev/ttyO5", O_RDWR | O_NOCTTY)) < 0) {
        fprintf(stderr,"Unable to open GPS_UART access in gps_novatel.c\n");
    }
    // GET PORT ATTRIBUTES
    if(tcgetattr(fd, &GPS_UART) < 0) {
        fprintf(stderr,"Could not get attributes of GPS_UART. Error in gps_novatel.c\n");
    }
    // SET BAUD RATE to B115200, 8 DATA BITS, NO PARITY, 1 STOP BIT
    if (cfsetospeed(&GPS_UART, B115200) < 0) {
        fprintf(stderr,"Could not set baud rate\n. gps_novatel.c");
    }
    else {
        fprintf(stderr,"\n\nBaud rate: 115200\n");
    }
    
    // SET ATTRIBUTES
    GPS_UART.c_iflag = 0;   //input modes
    GPS_UART.c_oflag = 0;   //output modes
    GPS_UART.c_lflag = 0;   //local modes
    tcsetattr(fd, TCSANOW, &GPS_UART);
    
    // POPULATE STRUCT FIELDS
    gpsData_ptr->port = fd;             //port handle
    gpsData_ptr->badDataCounter = 0;    //bad data counter
    gpsData_ptr->posValFlag = 0;        //position valid
    gpsData_ptr->lastPosVal = 0;        //last position valid
    
    return;
}


void endian_swap(uint8_t *buf, int index, int count) {
    
    int i;
    uint8_t tmp;
    
    for (i=0;i<(count/2);i++) {
        tmp = buf[index+i];
        buf[index+i] = buf[index+count-i-1];
        buf[index+count-i-1] = tmp;
    }
    
    return;
}

unsigned int CRC32Value(int i) {
    
    int j;
    unsigned int ulCRC;
    ulCRC = i;
    
    for (j=8;j>0;j--) {
        if (ulCRC & 1) {
            ulCRC = (ulCRC >> 1)^CRC32_POLYNOMIAL;
        }
        else {
            ulCRC >>= 1;
        }
    }
    
    return ulCRC;
}


unsigned int CalculateBlockCRC32(unsigned int ulCount, unsigned char *ucBuffer) {
    
    unsigned int ulTemp1;
    unsigned int ulTemp2;
    unsigned int ulCRC = 0;
    
    while (ulCount-- != 0) {
        ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
        ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
        ulCRC = ulTemp1 ^ ulTemp2;
    }
    
    return ulCRC ;
}


int read_GPS(struct gps *gpsData_ptr) {
    
    unsigned int state = 0;
    unsigned int counter = 0;
    
    int i = 0;
    int pCount = 0;
    int bytesInLocalBuffer;
    
    unsigned long CRC_computed;
    unsigned long CRC_read;
    unsigned int CRC_readstr[4];
    
    // READ 512 BYTES FROM THE SERIAL PORT
    bytesInLocalBuffer = read(gpsData_ptr->port, response, 512);
    
    // PRINT 144 BYTES FROM THE RESPONSE BUFFER (DEBUGGING)
    //for(i=0;i<144;i++) {
    //    fprintf(stderr,"%x, ",response[i]);
    //}
    
    // EXIT IN THE CASE OF INCOMPLETE RESPONSE
    if(bytesInLocalBuffer<144) {
        fprintf(stderr, "gps port returned less than 144 bytes: exiting read_GPS()\n");
        return -1;
    }
    
    // RECEIVER CLOCK SYNCHRONIZATION
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
                // COMPUTE CRC
                CRC_read = *((unsigned long *)(&response[140]));
                response[139] = '\0';
                CRC_computed = CalculateBlockCRC32(140,response+counter-2);
                //fprintf(stderr,"CRC Calculated = %x, CRC read = %x\n",CRC_computed,CRC_read);
                
                if (CRC_computed != CRC_read) {
                    
                    state = 0;
                    counter = 0;
                    fprintf(stderr,"CRC ERROR: CORRUPTED PACKET\n");
                    gpsData_ptr->badDataCounter++;
                    
                    if(gpsData_ptr->badDataCounter > 5) {
                        gpsData_ptr->badDataCounter = 0;
                    }
                    
                    return -1;
                }
                
                memcpy(gpsData_ptr->responseBuffer,&response[counter-2],144);//copy data read from gps port to gps struct

                /* NEED TO REVIEW SYSTEM OPERATION FOR GPS RESET AND TIMING
                //
                //
                //
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
                //
                //
                //
                */

                
                
                pCount = counter - 2;
                
                
                
                /* MAY OR MAY NOT NEED TO IMPLEMENT AN ENDIAN SWAP (DEPENDS ON THE MAIN PROCESSOR)
                //
                //
                //
                //endian_swap(response,pCount+14,2);
                //endian_swap(response,pCount+16,4);
                //endian_swap(response,pCount+28+8,8);
                //endian_swap(response,pCount+28+16,8);
                //endian_swap(response,pCount+28+24,8);
                //gpsData_ptr->timeStatus = *((uint8_t *)(&response[13]));
                //printf("GPS Time Status: %d\n",gpsData_ptr->timeStatus);
                //gpsData_ptr->weekRef = *((uint16_t *)(&response[14]));
                //
                //
                //
                */
                
                 
                // TIME STATUS
                gpsData_ptr->timeStatus = *((uint8_t *)(response + 13));
                
                // GPS WEEK REFERENCE NUMBER
                gpsData_ptr->weekRef = *((uint16_t *)(response + 14));
                
                // NUMBER OF SECONDS SINCE THE BEGINNING OF THE REFERENCE GPS WEEK
                gpsData_ptr->time = *((long *)(response + 16));

                
                /* NEED TO REVIEW SYSTEM OPERATION FOR GPS RESET AND TIMING
                //
                //
                //
                if(gpsData_ptr->timeStatus == 80 || gpsData_ptr->timeStatus == 100 || gpsData_ptr->timeStatus == 120 ||gpsData_ptr->timeStatus == 140 || gpsData_ptr->timeStatus == 160 || gpsData_ptr->timeStatus == 170 || gpsData_ptr->timeStatus == 180) {
                    //time statuses explained on page 27 of oemstar firmware manual
                    time_t unixSeconds = 315964800 + (24*7*3600*gpsData_ptr->weekRef + gpsData_ptr->time/1000);
                    // first number in above line is seconds between 1/1/70 and 1/6/80, or something like that
                    // gpsData_ptr->weekRef is the number of weeks since 1/6/80
                    // gpsData_ptr->time is "milliseconds into gps reference week" so time/1000 gives sec into gps ref week
                    
                    //time_t * timeSeconds = &unixSeconds;
                    //stime(timeSeconds);
                    stime(&unixSeconds);
                }
                //
                //
                //
                */
                
                
                // ECEF X
                gpsData_ptr->Xe = *((double *)(response + 36));
                
                // ECEF Y
                gpsData_ptr->Ye = *((double *)(response + 44));
                
                // ECEF Z
                gpsData_ptr->Ze = *((double *)(response + 52));
                
                
                // RESET THE STATE
                state = 0;
                
                // ZERO-OUT THE RESPONSE BUFFER AND BREAK THE LOOP
                bzero(response, response_length);
                counter = bytesInLocalBuffer;
                break;
                
        }
    }
    
    return 0;
}