<<<<<<< HEAD
// VN100.h
// This file facilitates interaction between the Tomcat
// and the VN100 IMU.
// Last Edited By: Luke Granlund
// Last Edited On: 18 June 2015, 16:00
=======
/*
 * vn100.h
 *
 *  Created on: Apr 3, 2015 at 7:52:10 PM
 *      Author: John
 */
>>>>>>> d447d3b6f73b9f3278e91ecbe7755a96794ee105

#ifndef VN100_H_
#define VN100_H_

<<<<<<< HEAD

#include "globaldefs.h"


#define IMU_PORT "/dev/ttyUSB0"             // TTY PORT TO VN100 (SET UP BY ftdi_sio KERNEL DRIVER)
#define IMU_DATAFILE "IMU_VN100.txt"        // Path to file where VN100 IMU date will be recorded


// Functions
int init_vn100();
unsigned char calculateChecksum(char*, size_t);
char *substring(char*, int, int)
int read_vn100(int, char*);


// VN100 Commands: Baud Rates //
=======
void reset_localBuffer();
unsigned char vn200_checksum_compute(char*);

// Serial Messages

// Baud Rates //
>>>>>>> d447d3b6f73b9f3278e91ecbe7755a96794ee105
unsigned char vn100_B9600[] = "$VNWRG,5,9600*60\n";
unsigned char vn100_B115200[] = "$VNWRG,5,115200*60\n";
unsigned char vn100_B230400[] = "$VNWRG,5,230400*6A\n";
unsigned char vn100_B460800[] = "$VNWRG,5,460800*65\n";
unsigned char vn100_B921600[] = "$VNWRG,5,921600*63\n";

<<<<<<< HEAD

// VN100 Commands: Async Commands //
=======
// ASync Commands //

>>>>>>> d447d3b6f73b9f3278e91ecbe7755a96794ee105
unsigned char vn100_async_none[] = "$VNWRG,6,0*5C\n";
unsigned char vn100_async_imu[] = "$VNWRG,6,19*64\n";
unsigned char vn100_async_ins[] = "$VNWRG,6,22*6C\n";

<<<<<<< HEAD

// VN100 Commands: IMU Read Requests //
unsigned char readCalibratedData[] = "$VNRRG,54*72\n";

=======
// GPS Read Requests //
unsigned char readCalibratedData[] = "$VNRRG,54*72\n";

// Init and read
int init_vn100();
int read_vn100(int,char*);
int write_VN100(char*);
unsigned char calculateChecksum(char*, size_t);
>>>>>>> d447d3b6f73b9f3278e91ecbe7755a96794ee105

#endif /* VN100_H_ */
