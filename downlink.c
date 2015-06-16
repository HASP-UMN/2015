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
#include "gps_novatel.h"
#include "spi.h"
#include "HASP_SPI_devices.h"
#include "downlink.h"

#define TELE_PACKET_SIZE 75
static int port;

void reset_string(char * string)
{
    int i;
    for(i = 0;i < 9; i++)
    {
        string[i]='\0';
    }
}

void init_telemetry()
{
    //uart1 configuration using termios
    struct termios uart1;
    //int fd = (int)malloc(sizeof(int));
    int fd;
    //open uart1 for tx/rx, not controlling device
    if((fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY)) < 0) {
        //fprintf(stderr,"Unable to open UART1 access.\n");}
    }
    //get attributes of uart1
    if(tcgetattr(fd, &uart1) < 0) {
        //fprintf(stderr,"Could not get attributes of UART1 at ttyO1\n");}
    }
    //set Baud rate
    if(cfsetospeed(&uart1, B1200) < 0) {
        //fprintf(stderr,"Could not set baud rate\n");}
    }
    else {
        //fprintf(stderr,"\n\nBaud rate: 1200\n");}
    }
    //set attributes of uart1
    uart1.c_iflag = 0;
    uart1.c_oflag = 0;
    uart1.c_lflag = 0;
    
    tcsetattr(fd, TCSANOW, &uart1);
    port = fd;
}

void send_telemetry(struct sensordata * sensorData)
{
    int bytes = 0;
    static char sendpacket[TELE_PACKET_SIZE];
    char * string_to_write = malloc(9*sizeof(char));
    reset_string(string_to_write);
    
    //header
    sprintf(string_to_write,"M,");
    memcpy(&sendpacket[0],string_to_write,2);
    reset_string(string_to_write);
    
    //gps time
    
    sprintf(string_to_write,"%u,",(uint32_t)sensorData->gpsData_ptr->time/1000);
    memcpy(&sendpacket[2],string_to_write,7);
    reset_string(string_to_write);
    
    //Xe
    sprintf(string_to_write,"%d,",(int)sensorData->gpsData_ptr->Xe);
    memcpy(&sendpacket[9],string_to_write,9);
    reset_string(string_to_write);
    
    //Ye
    sprintf(string_to_write,"%d,",(int)sensorData->gpsData_ptr->Ye);
    memcpy(&sendpacket[18],string_to_write,9);
    reset_string(string_to_write);
    
    //Ze
    sprintf(string_to_write,"%d,",(int)sensorData->gpsData_ptr->Ze);
    memcpy(&sendpacket[27],string_to_write,9);
    reset_string(string_to_write);
    
    //countsA
    sprintf(string_to_write,"%d,",sensorData->photonData_ptr->countsA);
    memcpy(&sendpacket[36],string_to_write,9);
    reset_string(string_to_write);
    //countsB
    
    sprintf(string_to_write,"%d,",sensorData->photonData_ptr->countsB);
    memcpy(&sendpacket[45],string_to_write,9);
    reset_string(string_to_write);
    
    //countsC
    sprintf(string_to_write,"%d,",sensorData->photonData_ptr->countsC);
    memcpy(&sendpacket[54],string_to_write,9);
    reset_string(string_to_write);

    //countsD
    sprintf(string_to_write,"%d,",sensorData->photonData_ptr->countsD);
    memcpy(&sendpacket[54],string_to_write,9);
    reset_string(string_to_write);

    
    //temp
    sprintf(string_to_write,"%.0f,",sensorData->imuData_ptr->temp_raw*0.14+25);
    memcpy(&sendpacket[63],string_to_write,5);
    reset_string(string_to_write);
    
    //supply
    sprintf(string_to_write,"%.1f",sensorData->imuData_ptr->supply_raw*0.002418);
    memcpy(&sendpacket[68],string_to_write,5);
    reset_string(string_to_write);
    
    //footer
    sprintf(string_to_write,"*\n");
    memcpy(&sendpacket[73],string_to_write,2);
    reset_string(string_to_write);
    
    while(bytes<TELE_PACKET_SIZE) {
        bytes += write(port, &sendpacket[bytes], 1);
    }
    free(string_to_write);
}