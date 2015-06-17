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
#include <sys/io.h>
#include <signal.h>

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

#DEFINE imu_stream_length 132;
char imu_data[imu_stream_length];

//Timestamps stored as longs
unsigned long t, t_0, log_period;
unsigned long imuStamp,gpsStamp,telStamp,eventStamp;

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
            /*
            if(!read_IMU(&imuData)){
                IMUlogger(&imuData, log_period);
            }
            */
            int fd = init_vn100();
            read_VN100(fd, buf);
            imuStamp = get_timestamp_ms();
            SMSTATE = IDLE;
            break;
        
        case RD_GPS:
            
            fprintf(stderr, "state = RD_GPS\n");
            /*
            if (!read_GPS(&gpsData) ){
                GPSlogger(&gpsData, log_period);
            }
            gpsStamp = get_timestamp_ms();
            */
            
            SMSTATE = IDLE;
            
            break;
            
        case DOWNLINK:
            
            fprintf(stderr, "state = DOWNLINK\n");
            /*
            send_telemetry(&sensorData);
            telStamp = get_timestamp_ms();
            */

            SMSTATE = IDLE;
            break;
            
        case EVENT_UPDATE:
            fprintf(stderr, "state = EVENT_UPDATE\n");
            /*
            updateEventCounter(&photonData);
            eventStamp = get_timestamp_ms();
            */
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
    t_0 = get_timestamp_ms();
    
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
    fprintf(stderr, "Would open fifo_dev here\n");
    
    storage_fd = open(storage_file_string, O_RDWR);
    unsigned char fifo_data_buf[BUFMAX];
    bzero(fifo_data_buf, BUFMAX);
    
    // next fork a child to call read on the fifo device in while(1)
    pid_t childpid;
    if ((childpid = fork()) == 0 )
    {
        //child code
        fprintf(stderr, "Child process: %i would enter read_fifo_store_data.c\n", childpid);
        read_fifo_store_data(fifo_fd, storage_fd, fifo_data_buf, BUFMAX);
    }
    
    //populate sensorData struct
    sensorData.gpsData_ptr = &gpsData;
    sensorData.imuData_ptr = &imuData;
    sensorData.photonData_ptr = &photonData;
    
    //init devices
    //init_GPS(&gpsData);
    //init_IMU(&imuData);
    //init_telemetry();
    
    init_IMU_new(&imuData);
    
    state SMSTATE = IDLE;
    
    sensorData.photonData_ptr->channel01 = 0;
    sensorData.photonData_ptr->channel02 = 0;
    sensorData.photonData_ptr->channel03 = 0;
    sensorData.photonData_ptr->channel04 = 0;
    
    usleep(500000); //not sure what this is for
    
    while (1)
    {
        t = get_timestamp_ms() - t_0;
        log_period = t/360000000000; //not sure what this is for
        SMSTATE = checkState(SMSTATE);
    }
    
    close_imu();
    
    // need to prob send a kill() to child process and then wait();
    kill(childpid, SIGTERM);
    
    return 0;
}


