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

    All of the errors for HASP are defined in the ERRWD enumeration in errorword.h with the highest
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


#define ERROR_FIFO "ErrorPipe.fifo"
#define FIFO_PERM (S_IRUSR | S_IWUSR)


unsigned short currentMaskedErrorID = 0;
unsigned short currentUnmaskedErrorID = 0;
unsigned short maskedErrors[10] = {0,0,0,0,0,0,0,0,0,0};

bool printErrors = true;


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
        fprintf(stderr,"############  HAXDT MAIN ERROR: %d  ############\n",errorID);  // For Debugging
    }
}


void reportErrorPipe(ERRWD errorID){
    int fd = open(ERROR_FIFO, O_WRONLY);
    if(fd == -1){
        return;
    }
    write(fd, &errorID, 2);
    if(printErrors==true){
        fprintf(stderr,"############  HAXDT PIPE ERROR: %d  ############\n",errorID);  // For Debugging
    }
    close(fd);
    return;
}


void readErrorPipe(){
    size_t bytesToRead;
    unsigned short errorID;
    bool currentPrintErrors = printErrors;
    fprintf(stderr,"Open fd\n");
    int fd = open(ERROR_FIFO, O_RDONLY);
    if(fd == -1){
        // report error
        fprintf(stderr,"Open failed\n");
        return;
    }

    printErrors = false;

    fprintf(stderr,"ioctl\n");
    if(ioctl(fd, FIONREAD, &bytesToRead) < 0){
        // report error
        fprintf(stderr,"ioctl failed\n");
        return;
    }
    fprintf(stderr, "Bytes To Read: %d\n", bytesToRead);

    while(bytesToRead > 0){
        if(read(fd, &errorID, 2) < 0){
            // report error
            return;
        }
        reportError(errorID);
        if(ioctl(fd, FIONREAD, &bytesToRead) < 0){
            //report error
            return;
        }
    }
    close(fd);

    printErrors = currentPrintErrors;
    return;
}


void getErrorWord(){
    // readErrorPipe();
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


void init_ErrorReporting(){

    struct stat fifostat;

    if(stat(ERROR_FIFO,&fifostat) == 0){
        if(unlink((ERROR_FIFO)) < 0){
            // report error
            return;
        }
    }

    if(mkfifo(ERROR_FIFO, FIFO_PERM) < 0){
        // report error
        return;
    }

    return;
}




