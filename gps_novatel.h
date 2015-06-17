#ifndef GPS_NOVATEL_H_
#define GPS_NOVATEL_H_

#include <stdint.h>
#include "globaldefs.h"

void init_GPS(struct gps *gpsData_ptr);
int read_GPS(struct gps *gpsData_ptr);
void endian_swap(uint8_t *buf, int index, int count);
unsigned int CRC32Value(int i);
unsigned int CalculateBlockCRC32(unsigned int ulCount, unsigned char *ucBuffer);

#endif
