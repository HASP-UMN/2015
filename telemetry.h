// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// telemetry.h
// This file send telemetry data out on COM2.
// Last Edited By: Luke Granlund
// Last Edited On: 14 July 2015, 16:00

#ifndef TELEMETRY_H_
#define TELEMETRY_H_

#include "globaldefs.h"

void init_telemetry();
void send_telemetry(struct imu *imuData, struct gps *gpsData, struct photons *photonData);

#endif
