#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
#include "errorword.h"


/*

    Important Resources for working with the NovAtel OEMStar GPS
    OEMStar Firmware Reference Manual:
    -- Binary Message Response Structure.... p. 25
    -- Time Status Specification............ p. 27
    -- BESTXYZ Log Specification............ p. 207
    -- Receiver Status Word Specification... p. 350

*/


#define GPS_PORT           "/dev/ttyUSB1"            // TTY to OEMStar GPS (Port for COM2)
#define GPS_DATAFILE       "GPS_OEMSTAR.raw"         // Path to file where OEMStar GPS data will be recorded

#define CRC32_POLYNOMIAL   0xEDB88320L

#define VALID_POS_OVERRIDE 1


unsigned char response[512];
FILE* GPSDataFile;  // This must be located in this file and not in main.c in order for the GPS data to be saved.


void init_GPS(struct gps *gpsData_ptr)
{

    // Set serial configuration using termios
    struct termios tty;
    int fd;

    // Open GPS serial port
    if((fd = open(GPS_PORT, O_RDWR | O_NOCTTY)) < 0) {
        fprintf(stderr,"GPS ERROR - Unable to open GPS port\n");
    }

    // Get attributes of GPS serial port
    if(tcgetattr(fd, &tty) < 0) {
        fprintf(stderr,"GPS ERROR - Could not get GPS port attributes\n");
    }

    // Set output baud rate (Output of Flight Computer)
    if(cfsetospeed(&tty, B115200) < 0) {
        fprintf(stderr,"GPS ERROR - Could not set output baud rate for GPS\n");
    }

    // Set input baud rate (Input of Flight Computer)
    if(cfsetispeed(&tty, B115200) < 0) {
        fprintf(stderr,"GPS ERROR - Could not set input baud rate for GPS\n");
    }

    // Set serial communication settings
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;


    if(tcsetattr(fd, TCSANOW, &tty) < 0) {
        fprintf(stderr,"GPS ERROR - Could not set port attributes\n");
    }


    gpsData_ptr->gps_fd = fd;

	return;
}

// Not Used
//void endian_swap(uint8_t *buf, int index, int count)
//{
//	int i;
//	uint8_t tmp;
//
//	for (i=0;i<(count/2);i++) {
//		tmp = buf[index+i];
//		buf[index+i] = buf[index+count-i-1];
//		buf[index+count-i-1] = tmp;
//	}
//	return;
//}

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


bool GetBitMask(unsigned char *bits, int startingByte, uint8_t bitMask)
{
    while(bitMask > 7)
    {
        bitMask -= 8;
        startingByte++;
    }
    return (bits[startingByte] >> bitMask) & 1;
}


void read_GPS(struct gps *gpsData_ptr)
{
    fprintf(stderr,"\n============= ENTER read_GPS() ===============\n"); // For Debugging

	unsigned int state = 0;
	unsigned int counter = 0;


	int pCount=0;
	int bytesInGPSBuffer;
	bool posValid = false;

	unsigned long CRC_computed;
	unsigned long CRC_read;
	unsigned int CRC_readstr[4];

	bytesInGPSBuffer = read(gpsData_ptr->gps_fd,&response[0],512);

    fprintf(stderr,"GPS - Bytes to Read: %d\n", bytesInGPSBuffer); // For Debugging

	if(bytesInGPSBuffer<144)
	{
	    if(bytesInGPSBuffer==-1){
            fprintf(stderr,"GPS ERROR - Bad read");
	    }
	    else{
           fprintf(stderr,"GPS ERROR - Not enough bytes\n");
	    }
		return;
	}


	while(counter<bytesInGPSBuffer){

        //fprintf(stderr,"0x%X ",resByte); // For Debugging

		switch(state) {
		case 0:
			if(response[counter]==0xAA){
				state++;
			}
			else{
				state = 0;
			}
			counter++;
			break;
		case 1:
			if(response[counter]==0x44){
				state++;
			}
			else{
				state = 0;
			}
            counter++;
			break;
		case 2:
			if(response[counter]==0x12){
				state++;
                fprintf(stderr,"GPS - Found Sync\n");
			}
			else{
				state = 0;
                counter++;
			}
			break;
		case 3:
		    pCount = counter - 2;

		    // Check to see if an entire packet can fit between the starting point and the end of the buffer.
		    if(bytesInGPSBuffer - pCount < 144){
                // Report Error
                fprintf(stderr, "GPS - ERROR - Partial Packet\n");
                return;
		    }

			CRC_read = *((unsigned long *)(&response[140 + pCount]));
			CRC_computed = CalculateBlockCRC32(140, response + pCount);
			if (CRC_computed != CRC_read) {
                // Report Error
				fprintf(stderr,"GPS - Checksum failed\n");      // For Debugging
				return;
			}


			gpsData_ptr->timeStatus = *((uint8_t *)(&response[13]));
			printf("GPS - Time Status: %d\n",gpsData_ptr->timeStatus);
			gpsData_ptr->weekRef = *((uint16_t *)(&response[14]));
			gpsData_ptr->time = *((long *)(&response[16]));


			// Set system time from GPS if it is good
            //if(gpsData_ptr->timeStatus == 80 || gpsData_ptr->timeStatus == 100 || gpsData_ptr->timeStatus == 120 ||gpsData_ptr->timeStatus == 140 || gpsData_ptr->timeStatus == 160 || gpsData_ptr->timeStatus == 170 || gpsData_ptr->timeStatus == 180)
			//{
			//	time_t unixSeconds = 315964800 + (24*7*3600*gpsData_ptr->weekRef + gpsData_ptr->time/1000);
			//	time_t *timeSeconds = &unixSeconds;
			//	fprintf(stderr,"GPS Unix Seconds: %d", timeSeconds);
			//	stime(timeSeconds);
			//}

            posValid = !GetBitMask(response,20 + pCount,19);
            gpsData_ptr->lastPosVal = posValid;

            if(GetBitMask(response,20 + pCount,0)){
                fprintf(stderr,"GPS ERROR - RSM - Error flag raised\n");
            }
            if(GetBitMask(response,20 + pCount,1)){
                fprintf(stderr,"GPS ERROR - RSM - Temperature warning\n");
            }
            if(GetBitMask(response,20 + pCount,2)){
                fprintf(stderr,"GPS ERROR - RSM - Voltage supply warning\n");
            }
            if(GetBitMask(response,20 + pCount,6)){
                fprintf(stderr,"GPS ERROR - RSM - Antenna shorted\n");
            }
            if(GetBitMask(response,20 + pCount,7)){
                fprintf(stderr,"GPS ERROR - RSM - CPU overloaded\n");
            }
            if(GetBitMask(response,20 + pCount,8)){
                fprintf(stderr,"GPS ERROR - RSM - COM1 buffer overrun\n");
            }
            if(GetBitMask(response,20 + pCount,15)){
                fprintf(stderr,"GPS ERROR - RSM - Auto Gain Control Warning 1\n");
            }
            if(GetBitMask(response,20 + pCount,17)){
                fprintf(stderr,"GPS ERROR - RSM - Auto Gain Control Warning 2\n");
            }
            if(GetBitMask(response,20 + pCount,18)){
                reportError(ERR_GPS_RSMALM);
                //fprintf(stderr,"GPS ERROR - RSM - Almanac/UTC invalid\n");
            }
            if(!posValid){
                reportError(ERR_GPS_RSMPOS);
                //fprintf(stderr,"GPS ERROR - RSM - No position solution\n");
            }
            if(GetBitMask(response,20 + pCount,22)){
                reportError(ERR_GPS_RSMCLO);
                //fprintf(stderr,"GPS ERROR - RSM - Clock model invalid\n");
            }
            if(GetBitMask(response,20 + pCount,24)){
                fprintf(stderr,"GPS ERROR - RSM - Software resource warning\n");
            }

//            fprintf(stderr,"GPS - RSM - Error Flag Raised..........: %d\n",GetBitMask(response,20 + pCount,0));
//            fprintf(stderr,"GPS - RSM - Temperature Warning........: %d\n",GetBitMask(response,20 + pCount,1));
//            fprintf(stderr,"GPS - RSM - Voltage Supply Warning.....: %d\n",GetBitMask(response,20 + pCount,2));
//            fprintf(stderr,"GPS - RSM - Antenna Shorted............: %d\n",GetBitMask(response,20 + pCount,6));
//            fprintf(stderr,"GPS - RSM - CPU Overloaded.............: %d\n",GetBitMask(response,20 + pCount,7));
//            fprintf(stderr,"GPS - RSM - COM1 Buffer Overrun........: %d\n",GetBitMask(response,20 + pCount,8));
//            fprintf(stderr,"GPS - RSM - Auto Gain Control Warning 1: %d\n",GetBitMask(response,20 + pCount,15));
//            fprintf(stderr,"GPS - RSM - Auto Gain Control Warning 2: %d\n",GetBitMask(response,20 + pCount,17));
//            fprintf(stderr,"GPS - RSM - Almanac/UTC Invalid........: %d\n",GetBitMask(response,20 + pCount,18));
//            fprintf(stderr,"GPS - RSM - No Position Solution.......: %d\n",!posValid);
//            fprintf(stderr,"GPS - RSM - Clock Model Invalid........: %d\n",GetBitMask(response,20 + pCount,22));
//            fprintf(stderr,"GPS - RSM - Software Resource Warning..: %d\n",GetBitMask(response,20 + pCount,24));




			if(posValid == false && VALID_POS_OVERRIDE == 0){
                gpsData_ptr->Xe = 0;
                gpsData_ptr->Ye = 0;
                gpsData_ptr->Ze = 0;
                // Report Error
                fprintf(stderr,"GPS - ERROR - Position not valid\n");
                return;
			}


			gpsData_ptr->Xe = *((double *)(&response[36]));
			gpsData_ptr->Ye = *((double *)(&response[44]));
			gpsData_ptr->Ze = *((double *)(&response[52]));

            // Write to GPS data file
            GPSDataFile = fopen(GPS_DATAFILE,"a");
            if(GPSDataFile==NULL){
                // Report Error
                fprintf(stderr,"GPS - ERROR - Could not open GPS data file\n");
                return;
            }
            if(fwrite(response + pCount,1,144,GPSDataFile)!=144){
                // Report Error
                fprintf(stderr,"GPS - ERROR - Could not successfully write to GPS data file\n");
            }
            fflush(GPSDataFile);
            fclose(GPSDataFile);

            fprintf(stderr,"============= EXIT read_GPS() ===============\n\n");
            return;
			break;
		}
	}

    fprintf(stderr,"GPS - ERROR - No Sync Found\n");
    fprintf(stderr,"============= EXIT read_GPS() ===============\n\n");
	return;

}




