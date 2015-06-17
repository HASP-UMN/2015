#include <stdio.h>       // IO functions
#include <stdlib.h>      // Standard C library
#include <errno.h>       // Error enumerations
#include <termios.h>     // Serial port I/O
#include <string.h>      // Character array operations
#include <math.h>        // Mathematical operations
#include <time.h>        // Time/date functions (e.g. time since UNIX epoch)
#include <fcntl.h>       // File descriptor control
#include <unistd.h>      // Symbolic constants and types
#include <sys/types.h>   // C data types
//                       //
//    HAXDT-SPECIFIC     //
//     HEADER FILES:     //
#include "globaldefs.h"  // Globally-defined variables (ports, buffer sizes, enumerated types/structs, etc.)


struct imu imuData;        // 'imu' struct for VN100 data
struct gps gpsData;        // 'gps' struct for OEMstar data
struct photons photonData; // 'photons' struct for digital data from the A/D converter


// Timestamps stored as longs
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
            if ( (time - imuStamp) >= 5){
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
    //t_0 = get_timestamp(); // from pre-processing/timing board
    
    // Initialize GPS receiver
    init_GPS(&gpsData);

    // Initialize IMU
    init_vn100(&imuData);

    // Initialize telemtry stream
    init_telemetry();
    
    state SMSTATE = IDLE;
    photonData.ch01 = 0.00;
    photonData.ch02 = 0.00;
    photonData.ch03 = 0.00;
    photonData.ch04 = 0.00;
    
    usleep(500000);
    
    while (1)
    {
        t = get_timestamp_ms() - t_0;
        log_period = t/360000000000;
        SMSTATE = checkState(SMSTATE);
    }
    
    close_imu();
    return 0;
}


