// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// telepipe.c

#include <stdlib.h>      // Standard C Library
#include <stdio.h>       // Standard I/O Library
#include <stdbool.h>     // Standard C Library for Boolean Capability
#include <math.h>        // Mathematical operations
#include <sys/stat.h>    // Named Pipes
#include <fcntl.h>       // I/O
#include <sys/ioctl.h>   // I/O
#include <unistd.h>      // Named Pipes

#include "globaldefs.h"
#include "telepipe.h"


int fdTelemetryPipe;


void recordChannel(unsigned char channel){
    if(fdTelemetryPipe > -1){
        write(fdTelemetryPipe, &channel, 2);
    }
    else{
        fprintf(stderr,"############  TELEMETRY PIPE NOT OPEN  #############\n");  // For Debugging
    }
    return;
}


void init_telemetryPipe(){
    fdTelemetryPipe = open(TELEMETRY_FIFO, O_WRONLY);
    return;
}





