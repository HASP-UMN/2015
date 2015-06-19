// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// VN100.c
// This file facilitates interaction between the Tomcat
// and the VN100 IMU.
// Last Edited By: Luke Granlund
// Last Edited On: 18 June 2015, 16:00

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

#include "globaldefs.h"
#include "serial.h"
#include "VN100.h"


// Initializes the file descriptor to read and write to the VN100 IMU.
int init_vn100(){

	// Creates file descriptor to read VN100 IMU
	int fd = open(IMU_PORT, O_RDWR | O_NOCTTY);

	if (fd < 0){
		return -1;
		}//endif

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
    	}//end for loop
    return xor;
}// end calculateChecksum


// Gets a substring of a character array.
char *substring(char* fullString, int start, int end) {
    static char shortString[11];
    memcpy(shortString, &fullString[start], end - start + 1);
    shortString[end - start + 1] = '\0';
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
	}//endif

	fprintf(stderr,"bytesToRead: %d\n",bytesToRead);//for error checking

    // Reponses from the VN100 IMU should contain 115 or 131 bytes. If there are
    // a different number of bytes in the response, it is invalid.
	if (bytesToRead != 115 && bytesToRead != 131){
        return 0;
    	}//endif

    // Reads data from the IMU.
	if (read(fd, &imuData_ptr->dataBuffer[0], bytesToRead) < 0){
        return 0;
	}//endif

    // If 131 bytes are returned from the IMU, then two responses were sent back, a small 16 byte response
    // at the beginning caused by the init_vn100() function and the normal 115 byte response. This loops takes
    // the 115 by response and shifts it to the beginning of the dataBuffer arrary, overwriting the 16 byte response.
	if (bytesToRead == 131){
		bytesToRead = 115;
		for(idx = 0; idx < bytesToRead; idx++){
			imuData_ptr->dataBuffer[idx] = imuData_ptr->dataBuffer[idx + 16];
        }//endif
    }

    // Checksum
	unsigned char Checksum = calculateChecksum(imuData_ptr->dataBuffer,bytesToRead);                                    // Calculated Checksum
	unsigned char bufCheck[] = {imuData_ptr->dataBuffer[bytesToRead - 4], imuData_ptr->dataBuffer[bytesToRead - 3]};    // Received Checksum
	// Compares the calculated checksum to received checksum
	if (Checksum != strtol(bufCheck, NULL, 16)){
		return 0;
    	}//endif

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
    imuData_ptr->Pressure = (float)atof(substring(imuData_ptr->dataBuffer, 100, 109));

    // Write IMU VN100 data to a file
	FILE *VN100File = fopen(IMU_DATAFILE,"a");
    for (idx = 0; idx < (bytesToRead - 1); idx++){
        //fprintf(stderr, "%c", imuData_ptr->dataBuffer[idx]);
        fprintf(VN100File, "%c", imuData_ptr->dataBuffer[idx]);
    	}//end forloop

	fflush(VN100File);
	fclose(VN100File);

    return 1;
}//end read_vn100

