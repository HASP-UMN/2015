/*
 * vn100.h
 *
 *  Created on: Apr 3, 2015 at 7:52:10 PM
 *      Author: John
 */

#ifndef VN100_H_
#define VN100_H_

void reset_localBuffer();
unsigned char vn200_checksum_compute(char*);

// Serial Messages

// Baud Rates //
unsigned char vn100_B9600[] = "$VNWRG,5,9600*60\n";
unsigned char vn100_B115200[] = "$VNWRG,5,115200*60\n";
unsigned char vn100_B230400[] = "$VNWRG,5,230400*6A\n";
unsigned char vn100_B460800[] = "$VNWRG,5,460800*65\n";
unsigned char vn100_B921600[] = "$VNWRG,5,921600*63\n";

// ASync Commands //

unsigned char vn100_async_none[] = "$VNWRG,6,0*5C\n";
unsigned char vn100_async_imu[] = "$VNWRG,6,19*64\n";
unsigned char vn100_async_ins[] = "$VNWRG,6,22*6C\n";

// GPS Read Requests //
unsigned char readCalibratedData[] = "$VNRRG,54*72\n";

// Init and read
int init_vn100();
int read_vn100(int,char*);
int write_VN100(char*);
unsigned char calculateChecksum(char*, size_t);

#endif /* VN100_H_ */
