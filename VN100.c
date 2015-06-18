// Test file for VN100 with tomcat
// Written by Charles Denis, 6/12/2015
// This .c defines functions for the communication with the VN100 IMU.
// Written specifically for the VN100

// Edited 2:26PM JUNE 16th - J.D.

// Include files
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "../utilities/serial/serial.h"
#include "VN100.h"

// TTY PORT TO VN100 (SET UP BY ftdi_sio KERNEL DRIVER)
#define PORT "/dev/ttyUSB0"

int init_vn100(){
	//fprintf(stderr,"ENTERED init_vn100\n");//for error checking
	int fd = open(PORT, O_RDWR | O_NOCTTY);
	//fprintf(stderr,"init_vn100: File Descriptor Created.\n");
	if (fd < 0){
		//fprintf(stderr,"Failed to open port!\n");//for error checking
		return -1;
		}
	//fprintf(stderr,"init_vn100: File Descriptor is valid.\n");
	set_interface_attribs(fd, B115200, 0);
	//fprintf(stderr,"init_vn100: set_interface_attribs was called successfully.\n");
	set_blocking(fd, 0);
	//fprintf(stderr,"init_vn100: set_blocking was called successfully.\n");
	write(fd, vn100_async_none, sizeof(vn100_async_none)); // Turn off the async data
	//fprintf(stderr, "WRITE ASYNC SUCCESSFUL\n");

	//fprintf(stderr, "EXITING init_vn100\n");
	//tcflush(fd,TCIOFLUSH);
	return fd;
}//end init_vn100()

unsigned char calculateChecksum(char* command, size_t length)
{
        int i;
        unsigned char xor = 0;
        for (i = 1; i < length-5; i++){
                xor ^= (unsigned char)command[i];
                }
        return xor;
}// end calculateChecksum


int read_vn100(int fd, char* dataBuf){

//	fprintf(stderr,"\n\n\n\n");
	fprintf(stderr,"ENTERING read_vn100()\n");
	size_t bytesToRead;
	//fprintf(stderr,"bytesToRead declared as size_t\n");
	int i;
	//fprintf(stderr,"i declared as int\n");

	//fprintf(stderr,"WRITING CALIBRATED DATA\n");
	write(fd, &readCalibratedData[0], sizeof(readCalibratedData));
	//fprintf(stderr,"WRITING OF CALIBRATED DATA SUCCESSFUL\n");//for error checkin

	//fprintf(stderr,"BEFORE I/O CONTROL TRANSFER\n");
	ioctl(fd, FIONREAD, &bytesToRead);
	//fprintf(stderr,"AFTER I/O CONTROL TRANSFER\n");

	fprintf(stderr,"bytesToRead: %d\n",bytesToRead);//for error checking
	if (bytesToRead == 0){
		//fprintf(stderr,"bytesToRead = 0. returning......\n");
		return 0;}

	if (bytesToRead != 115 && bytesToRead != 131){
                return 0;
                }

	//fprintf(stderr,"BEFORE read() CALL\n");
	read(fd, &dataBuf[0], bytesToRead);
	//fprintf(stderr,"READ SUCCESSFUL.\n");//for error checkin'
        fprintf(stderr,"\nBEGIN BUFFER\n");

	if (bytesToRead == 131){
		bytesToRead = 115;
		for(i=0;i<bytesToRead;i++){
			dataBuf[i] = dataBuf[i+16];
			}
		}

	unsigned char Checksum = calculateChecksum(dataBuf,bytesToRead);
	unsigned char bufCheck[] = {dataBuf[bytesToRead - 4], dataBuf[bytesToRead - 3]};

	if (Checksum != strtol(bufCheck,NULL,16)){
		return 0;
		}

	FILE *VN100File = fopen("VN100_test.txt","a");
	//fputs(dataBuf,VN100File);
        for (i=0; i<(bytesToRead-1);i++){
                fprintf(stderr,"%c",dataBuf[i]);
		fprintf(VN100File,"%c",dataBuf[i]);
                }
	//fprintf(stderr,"%c",calculateChecksum(dataBuf,bytesToRead));
	fflush(VN100File);
	fclose(VN100File);
        fprintf(stderr,"END BUFFER\n");
	return 1;
}//end read_vn100

