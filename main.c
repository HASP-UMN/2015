//
//  main.c
//  hasp
//
//  Created by Aron Lindell on 6/8/15.
//  Copyright (c) 2015 Aron Lindell. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

#include "globaldefs.h"
#include "datalogger.c"
#include "timing.c"
#include "updateEventCounter.c"
#include "downlink.c"
#include "HASP_SPI_devices.c"

//Data structures
struct sensordata sensorData;
struct imu imuData;
struct gps gpsData;
struct photons photonData;

//Timestamps stored as longs
unsigned long t, t_0, log_period;
unsigned long imuStamp,gpsStamp,telStamp,eventStamp;

//state machine state
typedef enum state{
    IDLE, RD_PEAK, RD_IMU, RD_GPS, DOWNLINK, EVENT_UPDATE, LOG_DATA
}
state;

state checkState(state SMSTATE)
{
    long time = get_timestamp_ms();
    
    switch (SMSTATE) {
        case IDLE:
            if ( (time - imuStamp) >= 30){
                SMSTATE = RD_IMU;
            }
            else if ( (time - gpsStamp) >= 1000){
                SMSTATE = RD_GPS;
            }
            else if ( (time - telStamp) >= 5000){
                SMSTATE = DOWNLINK;
            }
            else if ( (time - eventStamp) >= 5000){
                SMSTATE = EVENT_UPDATE;
            }
            else
                SMSTATE = IDLE;
            
            break;
            
        case RD_IMU:
            fprintf(stderr, "state = RD_IMU\n");
            if(!read_IMU(&imuData)){
                IMUlogger(&imuData, log_period);
            }
            imuStamp = get_timestamp_ms();
            SMSTATE = IDLE;
            break;
        
        case RD_GPS:
            fprintf(stderr, "state = RD_GPS\n");
            if (!read_GPS(&gpsData) ){
                GPSlogger(&gpsData, log_period);
            }
            gpsStamp = get_timestamp_ms();
            SMSTATE = IDLE;
            break;
            
        case DOWNLINK:
            fprintf(stderr, "state = DOWNLINK\n");
            send_telemetry(&sensorData);
            telStamp = get_timestamp_ms();
            SMSTATE = IDLE;
            break;
            
        case EVENT_UPDATE:
            fprintf(stderr, "state = EVENT_UPDATE\n");
            updateEventCounter(&photonData);
            eventStamp = get_timestamp_ms();
            SMSTATE = IDLE;
            break;
            
        default:
            fprintf(stderr, "state = default\n");
            SMSTATE = IDLE;
            break;
    }
    
    return SMSTATE;
    
}

int main(int argc, const char * argv[])
{
    t_0 = get_timestamp_ms();
    
    //populate sensorData struct
    sensorData.gpsData_ptr = &gpsData;
    sensorData.imuData_ptr = &imuData;
    sensorData.photonData_ptr = &photonData;
    
    //init devices
    init_GPS(&gpsData);
    init_IMU(&imuData);
    init_telemetry();
    
    state SMSTATE = IDLE;
    
    sensorData.photonData_ptr->countsA = 0;
    sensorData.photonData_ptr->countsB = 0;
    sensorData.photonData_ptr->countsC = 0;
    sensorData.photonData_ptr->countsD = 0;
    
        usleep(500000); //not sure what this is for
    
    while (1)
    {
        t = get_timestamp_ms() - t_0;
        log_period = t/360000000000; //not sure what this is for
        SMSTATE = checkState(SMSTATE);
    }
    
    close_imu();
    return 0;
}


