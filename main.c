//  ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
//  main.c
//  hasp
//
//  Created by Aron Lindell on 6/8/15.
//  Edited 6/19/2015 - Charlie Denis

// Include files
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
#include <sys/io.h>
#include <signal.h>
#include <sys/wait.h>

#include "globaldefs.h"
#include "read_fifo_store_data.h"
//#include "timing.h"
#include "VN100.h"
#include "gps_novatel.h"

//Data structures
struct imu imuData;
struct gps gpsData;
struct photons photonData;

// IMU
FILE* VN100File;
#define imu_stream_length 131
char imu_data[imu_stream_length];

// GPS
#define GPS_PORT "/dev/ttyS1"

//Timestamps stored as longs
unsigned long t, t_0;
unsigned long imuStamp,gpsStamp,telStamp,eventStamp;
int imu_fd;

// buffer for reading from photon data fifo
#define 		     PHOTON_BUF_MAX 500
unsigned char        PHOTON_DATA_BUFFER[PHOTON_BUF_MAX];
#define              BYTES_PER_PHOTON 10
int                  PHOTONS_AQUIRED = 0;

// ISA BUS INPUT PORT
const unsigned short INPUT_PORT = 0x800; // base address
#define SYNC_BYTE 77 //arbitrarily chosen for now
#define IRQ_NUM 6

//state machine state
typedef enum state{
    IDLE, RD_IMU, RD_GPS, DOWNLINK, EVENT_UPDATE, LOG_DATA
}
state;

state checkState(state SMSTATE)
{
   long time = get_timestamp_ms();

    switch (SMSTATE) {
        case IDLE:
            if ( (time - imuStamp) >= 500){
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
            /*
            if(!read_IMU(&imuData)){
                IMUlogger(&imuData, log_period);
            }
            */
            read_vn100(imu_fd, &imuData, VN100File);
            imuStamp = get_timestamp_ms();
            SMSTATE = IDLE;
            break;

        case RD_GPS:

            fprintf(stderr, "state = RD_GPS\n");

            if (!read_GPS(&gpsData) ){
                //GPSlogger(&gpsData, log_period);
            }

            gpsStamp = get_timestamp_ms();

            SMSTATE = IDLE;
            break;

        case DOWNLINK:

            fprintf(stderr, "state = DOWNLINK\n");
	    //send_telemetry(&sensorData);
            telStamp = get_timestamp_ms();

            SMSTATE = IDLE;
            break;

        case EVENT_UPDATE:
            fprintf(stderr, "state = EVENT_UPDATE\n");
            /*
            updateEventCounter(&photonData);
            */
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

int main()
{

    // Initialize IMU
	imu_fd = init_vn100();
	VN100File = fopen(IMU_DATAFILE,"a");

    // Initialize GPS
	init_GPS(&gpsData);
	gpsData.GPSDataFile = fopen("","a");

// add init for gps and telemetry here
	t_0 = get_timestamp_ms();
//    t = get_timestamp_ms() - t_0; // pointless
	eventStamp = t_0;
	gpsStamp = t_0;
	imuStamp = t_0;
	telStamp = t_0;

    photonData.counts_ch01 = 0;
    photonData.counts_ch02 = 0;
    photonData.counts_ch03 = 0;
    photonData.counts_ch04 = 0;


    // next fork a child to call read on the fifo device in while(1)
/*
    pid_t childpid;
    if ( (childpid = fork()) < 0)
	{
		fprintf(stderr, "Fork failed\n");
	}

  	if (childpid == 0 )
    {
        //child code
		execl("./read_fifo_store_data", "read_fifo_store_data", (char*) 0);
		fprintf(stderr, "Child failed to execl command\n");
		return -1;
	}
*/
    state SMSTATE = IDLE;
	int child_status;
    while (1)
    {
        t = get_timestamp_ms() - t_0;
        SMSTATE = checkState(SMSTATE);
	/*	if ( waitpid(childpid, &child_status, WNOHANG) == childpid)// just for testing, not flight code
		{
			break;
		} */
    }

    // Close IMU Data File
	fclose(VN100File);

	// Close GPS Data File
	fclose(gpsData.GPSDataFile);

    // need to prob send a kill() to child process and then wait();

//	 kill(childpid, SIGTERM);
//    fprintf(stderr, "Sent SIGTERM signal to child process %i : Now waiting for it\n", childpid);
//   waitpid(childpid, &child_status, WNOHANG);
    return 0;
}
