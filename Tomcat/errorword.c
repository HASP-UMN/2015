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
#include <unistd.h>

#include "globaldefs.h"
#include "errorword.h"


/*

    All of the errors for HASP are defined in the ERRWD enumeration in globaldefs.h with the highest
    priority errors at the top of the enumeration. When these errors occur in the application, they
    are reported with the reportError(ERRWD) method. The reportError(ERRWD) functions checks to see
    if a higher priority error has already been reported, and if not, it records the current error.
    When getErrorWord is called, the highest priority error reported error is returned. Once an
    error is used by getErrorWord, it is added to the maskedErrors array through the
    advanceErrorMask(errorID) method. If there is a situation where there are repeatedly multiple
    errors at the same time, the lower priority errors can also be shown in the error word. When
    reportError(ERRWD) tracks errors, it tracks the highest priority unmasked error and the highest
    priority masked error. If there are no unmasked errors (such that all of the current errors
    being reported are masked), the highest priority masked error that was reported since the last
    time getErrorWord() was called is used to form the error word and the maskedErrors array is
    cleared. The mask can hold up to ten of the most recent errors.

*/


unsigned short currentMaskedErrorID = 0;
unsigned short currentUnmaskedErrorID = 0;
unsigned short maskedErrors[10] = {0,0,0,0,0,0,0,0,0,0};

int fdErrorPipe;

bool printErrors = true;


// Creates a fresh fifo for the named pipe
void init_ErrorReporting(){

    struct stat fifostat;

    // If named pipe exists already, remove it
    if(stat(ERROR_FIFO,&fifostat) == 0){
        if(unlink((ERROR_FIFO)) < 0){
            reportError(ERR_ER_RMFIFO);
            return;
        }
    }

    // Create named pipe
    if(mkfifo(ERROR_FIFO, S_IRUSR | S_IWUSR) < 0){
        reportError(ERR_ER_MKFIFO);
        return;
    }

    return;
}


// Opens the named pipe
void init_ErrorPipe(){

    // Open named pipe
    fdErrorPipe = open(ERROR_FIFO, O_RDONLY);
    if(fdErrorPipe < 0){
        reportError(ERR_ER_OPIPE);
    }

    return;
}


void advanceErrorMask(unsigned int errorID){
    uint8_t idx;
    for(idx=9;idx>0;idx--){
        maskedErrors[idx] = maskedErrors[idx - 1];
    }
    maskedErrors[0] = errorID;
}


bool checkErrorMask(unsigned int errorID){
    uint8_t idx;
    for(idx=0;idx<10;idx++){
        if(maskedErrors[idx]==errorID){
            return true;
        }
    }
    return false;
}


void clearError(){
    currentMaskedErrorID = 0;
    currentUnmaskedErrorID = 0;
}


void clearErrorMask(){
    uint8_t idx;
    for(idx=0;idx<10;idx++){
        maskedErrors[idx] = 0;
    }
}


// For reporting an error
void reportError(ERRWD errorID){
    if(checkErrorMask(errorID)==false){
        if(errorID < currentUnmaskedErrorID || currentUnmaskedErrorID == 0){
            currentUnmaskedErrorID = errorID;
        }
    }
    else{
        if(errorID < currentMaskedErrorID || currentMaskedErrorID == 0){
            currentMaskedErrorID = errorID;
        }
    }
    if(printErrors==true){
        fprintf(stderr,"############  HAXDT MAIN ERROR: %X  ############\n",errorID);  // For Debugging
    }
}


// Reads in errors on the named pipe from the fifo process
void readErrorPipe(){
    size_t bytesToRead;
    unsigned short errorID;
    bool currentPrintErrors = printErrors;

    printErrors = false;

    if(ioctl(fdErrorPipe, FIONREAD, &bytesToRead) < 0){
        reportError(ERR_ER_PIPEBYTES);
        return;
    }

    while(bytesToRead > 1){
        if(read(fdErrorPipe, &errorID, 2) < 0){
            reportError(ERR_ER_RDPIPE);
            return;
        }
        reportError(errorID);
        if(ioctl(fdErrorPipe, FIONREAD, &bytesToRead) < 0){
            reportError(ERR_ER_PIPEBYTES);
            return;
        }
    }

    printErrors = currentPrintErrors;
    return;
}


// Gets Error Word for Telemetry
void getErrorWord(){
    readErrorPipe();
    if(currentUnmaskedErrorID!=0){
        ERROR_WORD = currentUnmaskedErrorID;
        advanceErrorMask(currentUnmaskedErrorID);
        clearError();
    }
    else if(currentMaskedErrorID!=0){
        ERROR_WORD = currentMaskedErrorID;
        clearErrorMask();
        advanceErrorMask(currentMaskedErrorID);
        clearError();
    }
    else{
        ERROR_WORD = 0;
    }
    return;
}






