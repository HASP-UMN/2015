//  ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
//  main.c
//  hasp
//
//  Created by Aron Lindell on 6/8/15.
//  Copyright (c) 2015 Aron Lindell. All rights reserved.
//  Edited 6/19/2015 - Charlie Denis


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
//#include "timing.c" // linked in make file
//#include "timing.h"
//#include "downlink.c"
//#include "VN100.c" // linked in make file

//Data structures
struct imu imuData;
struct gps gpsData;
struct photons photonData;
//#define imu_stream_length 132
//char imu_data[imu_stream_length];

//Timestamps stored as longs
unsigned long t, t_0;
unsigned long imuStamp,gpsStamp,telStamp,eventStamp;
int imu_fd;





// Definition moved here from globaldefs.h for error checking 6/19/2015 10am -Charlie
//extern const unsigned long  LENGTH = 500;
extern unsigned char        PHOTON_DATA_BUFFER[500];
extern int                  BYTES_PER_PHOTON = 10;
extern int                  PHOTONS_AQUIRED = 0;
// ISA BUS INPUT PORT
const unsigned short INPUT_PORT = 0x800; // base address
int SYNC_BYTE = 77; //arbitrarily chosen for now
int IRQ = 6;
// END OF COPIED DATA from globaldefs.h






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
            read_vn100(imu_fd, &imuData);
            imuStamp = get_timestamp_ms();
            SMSTATE = IDLE;
            break;

        case RD_GPS:

            fprintf(stderr, "state = RD_GPS\n");
            /*
            if (!read_GPS(&gpsData) ){
                GPSlogger(&gpsData, log_period);
            }
			*/
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
	imu_fd = init_vn100();
	t_0 = get_timestamp_ms();
        t = get_timestamp_ms() - t_0;
	eventStamp = t_0;
	gpsStamp = t_0;
	imuStamp = t_0;
	telStamp = t_0;

    /*
    fprintf(stderr,"SETTING PERMISSION TO ACCESS PORT AT 0x%x - 0x%lx",INPUT_PORT,INPUT_PORT+LENGTH);

    if(ioperm(INPUT_PORT,LENGTH+1,1)) {
        perror("ERROR");
        fprintf(stderr,"UNABLE TO SET PERMISSION TO ACCESS 0x%x - 0x%lx",INPUT_PORT,INPUT_PORT+LENGTH);
        return -1;
    }
     */

    // should open fifo_driver here. That will register the irq6 line
    int fifo_fd, storage_fd;
    char * storage_file_string = "fifo_data.txt";
    // fifo_fd = open("/dev/fifo_dev", O_RDONLY); //doesn't work yet
	fifo_fd = 0;
    fprintf(stderr, "Would open fifo_dev here\n");

    storage_fd = open(storage_file_string, O_RDWR);
    unsigned char fifo_data_buf[BUFMAX];
    bzero(fifo_data_buf, BUFMAX);

    // next fork a child to call read on the fifo device in while(1)

    pid_t childpid;
//    childpid = fork();
//	int count = 0;
//    if (childpid == 0 )
//    {
//        //child code
//	while(1){
//        //	fprintf(stderr, "Child process: %i would enter read_fifo_store_data.c\n", childpid);
//       		read_fifo_store_data(fifo_fd, storage_fd, fifo_data_buf, BUFMAX);
//	}
//    }
    //init devices
    //init_GPS(&gpsData);
    //init_IMU(&imuData);
    //init_telemetry();

    state SMSTATE = IDLE;

    photonData.counts_ch01 = 0;
    photonData.counts_ch02 = 0;
    photonData.counts_ch03 = 0;
    photonData.counts_ch04 = 0;

    usleep(500000); //not sure what this is for

    while (1)
    {
        t = get_timestamp_ms() - t_0;
        SMSTATE = checkState(SMSTATE);
    }

//    close_imu();

    // need to prob send a kill() to child process and then wait();
/*
   int child_status;
    kill(childpid, SIGTERM);
    fprintf(stderr, "Sent SIGTERM signal to child process %i : Now waiting for it\n", childpid);
    waitpid(childpid, &child_status, WNOHANG);
*/
    return 0;
}
