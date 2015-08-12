// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// gps_novatel.h
// This file reads from the NovAtel OEMStar GPS Receiver.
// Last Edited By: Luke Granlund
// Last Edited On: 13 July 2015, 13:00

#ifndef GPS_NOVATEL_H_
#define GPS_NOVATEL_H_

#include <stdint.h>
#include <stdbool.h>
#include "globaldefs.h"

void init_GPS(struct gps *gpsData_ptr);
void read_GPS(struct gps *gpsData_ptr);
int GetBitMask(unsigned char *bits, int startingByte, uint8_t bitMask);
unsigned int CRC32Value(int i);
unsigned int CalculateBlockCRC32(unsigned int ulCount, unsigned char *ucBuffer);

#endif

