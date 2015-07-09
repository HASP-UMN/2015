#ifndef GPS_NOVATEL_H_
#define GPS_NOVATEL_H_

#include <stdint.h>
#include <stdbool.h>
#include "globaldefs.h"

#define GPS_RST 61
#define GPS_POS_VAL 44

void init_GPS(struct gps *gpsData_ptr);
void read_GPS(struct gps *gpsData_ptr);
//void endian_swap(uint8_t *buf, int index, int count);    // Not Used
bool GetBitMask(unsigned char *bits, int startingByte, uint8_t bitMask);
unsigned int CRC32Value(int i);
unsigned int CalculateBlockCRC32(unsigned int ulCount, unsigned char *ucBuffer);

#endif

