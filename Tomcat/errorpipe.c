// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// errorword.c
// This file manages errors reported by the applications and
//     creates an error word to be sent in the telemetry packet.
// Last Edited By: Luke Granlund
// Last Edited On: 14 July 2015, 15:30

#include <stdlib.h>      // Standard C Library
#include <stdio.h>       // Standard I/O Library
#include <stdbool.h>     // Standard C Library for Boolean Capability
#include <math.h>        // Mathematical operations
#include <sys/stat.h>    // Named Pipes
#include <fcntl.h>       // I/O
#include <sys/ioctl.h>   // I/O
#include <unistd.h>      // Named Pipes

#include "globaldefs.h"
#include "errorpipe.h"


int fdErrorPipe;


void reportError(ERRWD errorID){
    if(fdErrorPipe > -1){
        write(fdErrorPipe, &errorID, 2);
    }
    else{
        fprintf(stderr,"############  ERROR PIPE NOT OPEN  #############\n",errorID);  // For Debugging
    }
    fprintf(stderr,"############  HAXDT PIPE ERROR: %X  ############\n",errorID);  // For Debugging
    return;
}


void init_ErrorPipe(){
    fdErrorPipe = open(ERROR_FIFO, O_WRONLY);
    return;
}




