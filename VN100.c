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

// VN100 Commands: Baud Rates //
const char* IMU_PORT = "/dev/ttyUSB0";             // TTY PORT TO VN100 (SET UP BY ftdi_sio KERNEL DRIVER)
const char* IMU_DATAFILE = "IMU_VN100.txt";        // Path to file where VN100 IMU date will be recorded

unsigned char vn100_B9600[] = "$VNWRG,5,9600*60\n";
unsigned char vn100_B115200[] = "$VNWRG,5,115200*60\n";
unsigned char vn100_B230400[] = "$VNWRG,5,230400*6A\n";
unsigned char vn100_B460800[] = "$VNWRG,5,460800*65\n";
unsigned char vn100_B921600[] = "$VNWRG,5,921600*63\n";

// VN100 Commands: Async Commands //
unsigned char vn100_async_none[] = "$VNWRG,6,0*5C\n";
unsigned char vn100_async_imu[] = "$VNWRG,6,19*64\n";
unsigned char vn100_async_ins[] = "$VNWRG,6,22*6C\n";

// VN100 Commands: IMU Read Requests //
unsigned char readCalibratedData[] = "$VNRRG,54*72\n";

struct imu* imuData_ptr;

// Initializes the file descriptor to read and write to the VN100 IMU.
int init_vn100(){

	// Creates file descriptor to read VN100 IMU
	int fd = open(IMU_PORT, O_RDWR | O_NOCTTY);

	if (fd < 0){
		fprintf(stderr, "Failed to open IMU port\n");
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
int read_vn100(int fd, struct imu* imuData_ptr, FILE* VN100File){
	fprintf(stderr,"ENTERING read_vn100()\n");

	size_t bytesToRead;
	int idx;

	// Sends a write register command to register #6 on the IMU.
	// This tells the IMU to send back the proper information.
	// See the VN100 User Manual for more information.
	write(fd, &readCalibratedData[0], sizeof(readCalibratedData));

	// Deterimes how many bytes to read back from the IMU. Exits on error.
	if (ioctl(fd, FIONREAD, &bytesToRead) < 0){
        return -1;
	}//endif

	fprintf(stderr,"bytesToRead: %d\n",bytesToRead);//for error checking

    // Reponses from the VN100 IMU should contain 115 or 131 bytes. If there are
    // a different number of bytes in the response, it is invalid.
	if (bytesToRead != 115 && bytesToRead != 131){
        return -1;
    	}//endif

    // Reads data from the IMU.
	if (read(fd, &imuData_ptr->dataBuffer[0], bytesToRead) < 0){
        return -1;
	}//endif

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
//	FILE *VN100File = fopen(IMU_DATAFILE,"a"); //should be opened and closed just once in main() rather than here
/*
    for (idx = 0; idx < (bytesToRead - 1); idx++){
        fprintf(stderr, "%c", imuData_ptr->dataBuffer[idx]);
        fprintf(VN100File, "%c", imuData_ptr->dataBuffer[idx]);
	}
*/
	fwrite(imuData_ptr->dataBuffer, sizeof(imuData_ptr->dataBuffer[0]), bytesToRead - 1, VN100File);
	fwrite(imuData_ptr->dataBuffer, sizeof(imuData_ptr->dataBuffer[0]), bytesToRead - 1, stderr);

// should automatically flush if we add \n to the end of each string to be written, will check on this
	fflush(VN100File);
//	fclose(VN100File);

    return 0;
}//end read_vn100

