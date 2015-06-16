#ifndef _DATALOGGER_H_
#define _DATALOGGER_H_

#include "globaldefs.h"


int GPSlogger(struct gps *gpsData_ptr, uint8_t log_number);
int IMUlogger(struct imu *imuData_ptr, uint8_t log_number);


#endif