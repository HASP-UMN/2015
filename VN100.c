<<<<<<< HEAD
// VN100.c
// This file facilitates interaction between the Tomcat
// and the VN100 IMU.
// Last Edited By: Luke Granlund
// Last Edited On: 18 June 2015, 16:00
=======
// Test file for VN100 with tomcat
// Written by Charles Denis, 6/12/2015
// This .c defines functions for the communication with the VN100 IMU.
// Written specifically for the VN100

// Edited 2:26PM JUNE 16th - J.D.
>>>>>>> d447d3b6f73b9f3278e91ecbe7755a96794ee105

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
<<<<<<< HEAD
#include "globaldefs.h"
#include "serial.h"
#include "VN100.h"


// Initializes the file descriptor to read and write to the VN100 IMU.
int init_vn100(){

	// Creates file descriptor to read VN100 IMU
	int fd = open(IMU_PORT, O_RDWR | O_NOCTTY);

	if (fd < 0){
		return -1;
    }

	// Sets up serial settings
	set_interface_attribs(fd, B115200, 0);
	set_blocking(fd, 0);

	// Sends command to the VN100 IMU to turn off the async data.
	write(fd, vn100_async_none, sizeof(vn100_async_none));

	return fd;
}//end init_vn100()


// Calculates Checksum from VN100 output. See VN100 User Manual for more information.
unsigned char calculateChecksum(char* command, size_t length){
    int idx;
    unsigned char xor = 0;
    for (idx = 1; idx < length-5; idx++){
        xor ^= (unsigned char)command[idx];
    }
    return xor;
}// end calculateChecksum


// Gets a substring of a character array.
char *substring(char* fullString, int start, int end) {
    static char shortString[];
    memcpy(shortString, &fullString[start], start - end + 1);
    shortString[end + 1] = '\0';
    return (char *)shortString;
}// end substring


// Reads the VN100 IMU, populates the IMU structure, and writes the data to a file.
int read_vn100(int fd, struct imu* imuData_ptr){
	fprintf(stderr,"ENTERING read_vn100()\n");

	size_t bytesToRead;
	int idx;

	// Sends a write register command to register #6 on the IMU.
	// This tells the IMU to send back the proper information.
	// See the VN100 User Manual for more information.
	write(fd, &readCalibratedData[0], sizeof(readCalibratedData));

	// Deterimes how many bytes to read back from the IMU. Exits on error.
	if (ioctl(fd, FIONREAD, &bytesToRead) < 0){
        return 0;
	}

	fprintf(stderr,"bytesToRead: %d\n",bytesToRead);//for error checking

    // Reponses from the VN100 IMU should contain 115 or 131 bytes. If there are
    // a different number of bytes in the response, it is invalid.
	if (bytesToRead != 115 && bytesToRead != 131){
        return 0;
    }

    // Reads data from the IMU.
	if (read(fd, &imuData_ptr->dataBuffer[0], bytesToRead) < 0){
        return 0;
	}

    // If 131 bytes are returned from the IMU, then two responses were sent back, a small 16 byte response
    // at the beginning caused by the init_vn100() function and the normal 115 byte response. This loops takes
    // the 115 by response and shifts it to the beginning of the dataBuffer arrary, overwriting the 16 byte response.
	if (bytesToRead == 131){
		bytesToRead = 115;
		for(idx = 0; idx < bytesToRead; idx++){
			imuData_ptr->dataBuffer[idx] = imuData_ptr->dataBuffer[idx + 16];
        }
    }

    // Checksum
	unsigned char Checksum = calculateChecksum(imuData_ptr->dataBuffer,bytesToRead);                                    // Calculated Checksum
	unsigned char bufCheck[] = {imuData_ptr->dataBuffer[bytesToRead - 4], imuData_ptr->dataBuffer[bytesToRead - 3]};    // Received Checksum
	// Compares the calculated checksum to received checksum
	if (Checksum != strtol(bufCheck, NULL, 16)){
		return 0;
    }

    // Writes the data to the IMU structure
    imuData_ptr->MagX = (float)atof(substring(imuData_ptr->dataBuffer, 10, 17));
    imuData_ptr->MagY = (float)atof(substring(imuData_ptr->dataBuffer, 19, 26));
    imuData_ptr->MagZ = (float)atof(substring(imuData_ptr->dataBuffer, 28, 35));
    imuData_ptr->AccelX = (float)atof(substring(imuData_ptr->dataBuffer, 37, 43));
    imuData_ptr->AccelY = (float)atof(substring(imuData_ptr->dataBuffer, 45, 51));
    imuData_ptr->AccelZ = (float)atof(substring(imuData_ptr->dataBuffer, 53, 59));
    imuData_ptr->GyroX = (float)atof(substring(imuData_ptr->dataBuffer, 61, 70));
    imuData_ptr->GyroY = (float)atof(substring(imuData_ptr->dataBuffer, 72, 81));
    imuData_ptr->GyroZ = (float)atof(substring(imuData_ptr->dataBuffer, 83, 92));
    imuData_ptr->Temp = (float)atof(substring(imuData_ptr->dataBuffer, 94, 98));
    imuData_ptr->Pressure =  = (float)atof(substring(imuData_ptr->dataBuffer, 100, 109));

    // Write IMU VN100 data to a file
	FILE *VN100File = fopen(IMU_DATAFILE,"a");
    for (idx = 0; idx < (bytesToRead - 1); idx++){
        //fprintf(stderr, "%c", imuData_ptr->dataBuffer[idx]);
        fprintf(VN100File, "%c", imuData_ptr->dataBuffer[idx]);
    }
	fflush(VN100File);
	fclose(VN100File);

    return 1;
=======
#include "serial.h"
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
	size_t bytesToRead=0;
	//fprintf(stderr,"bytesToRead declared as size_t\n");
	int i,ioctl_RETURN;
	//fprintf(stderr,"i declared as int\n");

	//fprintf(stderr,"WRITING CALIBRATED DATA\n");
	write(fd, &readCalibratedData[0], sizeof(readCalibratedData));
	//fprintf(stderr,"WRITING OF CALIBRATED DATA SUCCESSFUL\n");//for error checkin

	//fprintf(stderr,"BEFORE I/O CONTROL TRANSFER\n");
	ioctl_RETURN = ioctl(fd, FIONREAD, &bytesToRead);
	//fprintf(stderr,"AFTER I/O CONTROL TRANSFER\n");

	fprintf(stderr,"ioctl return: %d\n",ioctl_RETURN);
	fprintf(stderr,"bytesToRead: %d\n",bytesToRead);//for error checking
	if (bytesToRead == 0){
		//fprintf(stderr,"bytesToRead = 0. returning......\n");
		return 0;
		}
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
>>>>>>> d447d3b6f73b9f3278e91ecbe7755a96794ee105
}//end read_vn100

