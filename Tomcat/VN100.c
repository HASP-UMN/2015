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
#include "errorword.h"
#include "VN100.h"

// VN100 I/O
#define IMU_PORT "/dev/ttyUSB0"                // TTY PORT TO VN100 (SET UP BY ftdi_sio KERNEL DRIVER)
#define IMU_DATAFILE "data/IMU/IMU_VN100.txt"           // Path to file where VN100 IMU date will be recorded

// VN100 Commands: Baud Rates
unsigned char vn100_B115200[] = "$VNWRG,5,115200*60\n";

// VN100 Commands: Async Commands
unsigned char vn100_async_none[] = "$VNWRG,6,0*5C\n";

// VN100 Commands: IMU Read Requests
unsigned char readCalibratedData[] = "$VNRRG,54*72\n";

// Timing
struct timeval spec;
long stime;
long millitime;

// Checksum
unsigned char Checksum;
unsigned char bufCheck[2];


void send_read_command(struct imu* imuData_ptr){

	// Sends a read register command to IMU (Register 54)
	write(imuData_ptr->imu_fd, &readCalibratedData[0], sizeof(readCalibratedData));

    // Get time that VN100 makes a measurement
    gettimeofday(&spec, NULL);
    stime = spec.tv_sec;
    millitime = spec.tv_usec/1000;

    return;
}

// Initializes the file descriptor to read and write to the VN100 IMU.
void init_vn100(struct imu* imuData_ptr){

    /*

        Due to IMU/Tomcat behavior, there is a specific process to initialize
        the IMU. After a hard boot, the IMU buffer will not clear causing an
        infinitely increasing amount of bytes to read. To avoid this problem,
        the file descriptor is opened, the async response on the IMU is
        turned off, then the file descriptor is closed. After waiting one
        second, the file descriptor is opened again. This provides the chance
        for the buffer to be cleared and the IMU to turn off the async data
        before a read is made.

    */

	imuData_ptr->imu_fd = open(IMU_PORT, O_RDWR | O_NOCTTY);

	if (imuData_ptr->imu_fd < 0){
		reportError(ERR_IMU_PORTOPEN);
		return;
    }//endif

	// Sets up serial settings
	set_interface_attribs(imuData_ptr->imu_fd, B115200, 0);
	set_blocking(imuData_ptr->imu_fd, 0);

	// Sends command to the VN100 IMU to turn off the async data.
	write(imuData_ptr->imu_fd, vn100_async_none, sizeof(vn100_async_none));

    // Closes the file descriptor
    close(imuData_ptr->imu_fd);

    // Wait for one second
	usleep(1000000);

    // Reopen file descriptor to make reads
	imuData_ptr->imu_fd = open(IMU_PORT, O_RDWR | O_NOCTTY);
	if (imuData_ptr->imu_fd < 0){
		reportError(ERR_IMU_PORTOPEN);
		return;
    }

	// Sets up serial settings
	set_interface_attribs(imuData_ptr->imu_fd, B115200, 0);
	set_blocking(imuData_ptr->imu_fd, 0);

	// Set up file to write to.
	imuData_ptr->VN100File = fopen(IMU_DATAFILE,"a");
	if(imuData_ptr->VN100File==NULL){
        reportError(ERR_IMU_WRITEFO);
        return;
    }

    // The read command must be sent now so the IMU has data available when the
    // Tomcat does its first read. The IMU opeates at 200 Hz, so if the read command
    // is sent immediately before the Tomcat reads, there won't be data available yet.
    send_read_command(imuData_ptr);

	return;
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
void read_vn100(struct imu* imuData_ptr){

	size_t bytesToRead;
	int idx;

	// Deterimes how many bytes to read back from the IMU. Exits on error.
	if (ioctl(imuData_ptr->imu_fd, FIONREAD, &bytesToRead) < 0){
        return;
	}//endif

	//fprintf(stderr,"IMU - Bytes To Read: %d\n",bytesToRead);  // For Debugging

    // Reponses from the VN100 IMU should contain 115 or 131 bytes. If there are
    // a different number of bytes in the response, it is invalid.
	if (bytesToRead != 115){
        reportError(ERR_IMU_READBYTES);
        return;
    }//endif

    // Reads data from the IMU.
	if (read(imuData_ptr->imu_fd, &imuData_ptr->dataBuffer[0], bytesToRead) < 0){
        reportError(ERR_IMU_READ);
        return;
	}//endif

    // Checksum
	Checksum = calculateChecksum(imuData_ptr->dataBuffer,bytesToRead);     // Calculated Checksum
	bufCheck[0] = imuData_ptr->dataBuffer[bytesToRead - 4];                // Received Checksum
	bufCheck[1] = imuData_ptr->dataBuffer[bytesToRead - 3];
	// Compares the calculated checksum to received checksum
	if (Checksum != strtol(bufCheck, NULL, 16)){
        reportError(ERR_IMU_READCHECK);
		return;
    }//endif

    // Adds timestamp
    sprintf(imuData_ptr->dataBuffer + 113, ",T%d.%03d", stime, millitime);
    imuData_ptr->dataBuffer[129] = '\n';

    // Writes the data to the IMU structure
    imuData_ptr->Temp = (float)atof(substring(imuData_ptr->dataBuffer, 94, 98));

    // Write IMU VN100 data to a file
	fwrite(imuData_ptr->dataBuffer, sizeof(imuData_ptr->dataBuffer[0]), sizeof(imuData_ptr->dataBuffer), imuData_ptr->VN100File);
	//fwrite(imuData_ptr->dataBuffer, sizeof(imuData_ptr->dataBuffer[0]), bytesToRead - 1, stderr);  // For Debugging

    // Should automatically flush if we add \n to the end of each string to be written, will check on this
	fflush(imuData_ptr->VN100File);


	// The read command must be sent now so the IMU has data available when the
    // Tomcat does its next read. The IMU opeates at 200 Hz, so if the read command
    // is sent immediately before the Tomcat reads, there won't be data available yet.
    send_read_command(imuData_ptr);


    return;
}//end read_vn100

