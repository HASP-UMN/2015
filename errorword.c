// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// errorword.c
// This file manages errors reported by the applications and
//     creates an error word to be sent in the telemetry packet.
// Last Edited By: Luke Granlund
// Last Edited On: 13 July 2015, 13:00

#include <stdlib.h>      // Standard C Library
#include <stdio.h>       // Standard I/O Library
#include <stdbool.h>     // Standard C Library for Boolean Capability
#include <errno.h>       // System Error Enumerations
#include <math.h>        // Mathematical operations

#include "errorword.h"


/*

    All of the errors for HASP are defined in the ERRWD enumeration in errorword.h with the highest
    priority errors at the top of the enumeration. When these errors occur in the application, they
    are reported with the reportError(ERRWD) method. The reportError(ERRWD) functions checks to see
    if a higher priority error has already been reported, and if not, it records the current error
    and system errno. When getErrorWord is called, the highest priority error and its corresponding
    system errno (after a bitshift) are added together to form an error word. Once an error is used
    by getErrorWord, it is added to the maskedErrors array through the advanceErrorMask(errorID)
    method. If there is a situation where there are repeatedly multiple errors at the same time,
    the lower priority errors can also be shown in the error word. When reportError(ERRWD) tracks
    errors, it tracks the highest priority unmasked error and the highest priority masked error. If
    there are no unmasked errors (such that all of the current errors being reported are masked),
    the highest priority masked error that was reported since the last time getErrorWord() was
    called is used to form the error word and the maskedErrors array is cleared. The mask can hold
    up to ten of the most recent errors.

*/


unsigned int currentMaskedErrorID = 0;
unsigned int currentUnmaskedErrorID = 0;
unsigned int currentMaskedSysErrno = 0;
unsigned int currentUnmaskedSysErrno = 0;
unsigned int maskedErrors[10] = {0,0,0,0,0,0,0,0,0,0};


void advanceErrorMask(unsigned int errorID){
    int idx;
    for(idx=9;idx>0;idx--){
        maskedErrors[idx] = maskedErrors[idx - 1];
    }
    maskedErrors[0] = errorID;
}


bool checkErrorMask(unsigned int errorID){
    int idx;
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
    currentMaskedSysErrno = 0;
    currentUnmaskedSysErrno = 0;
}

void clearErrorMask(){
    int idx;
    for(idx=0;idx<10;idx++){
        maskedErrors[idx] = 0;
    }
}


void reportError(ERRWD errorID){
    if(checkErrorMask(errorID)==false){
        if(errorID < currentUnmaskedErrorID || currentUnmaskedErrorID == 0){
            currentUnmaskedErrorID = errorID;
            currentUnmaskedSysErrno = errno;
            errno = 0;
        }
    }
    else{
        if(errorID < currentMaskedErrorID || currentMaskedErrorID == 0){
            currentMaskedErrorID = errorID;
            currentMaskedSysErrno = errno;
            errno = 0;
        }
    }
    fprintf(stderr,"############  HASP ERROR: %d,  ERRNO: %d  ############\n",errorID,errno);  // For Debugging
}


int getErrorWord(){
    int errorWord = 0;
    int sysErrno = 0;
    if(currentUnmaskedErrorID!=0){
        sysErrno = currentUnmaskedSysErrno << 8;
        errorWord = sysErrno + currentUnmaskedErrorID;
        advanceErrorMask(currentUnmaskedErrorID);
        clearError();
    }
    else if(currentMaskedErrorID!=0){
        sysErrno = currentMaskedSysErrno << 8;
        errorWord = sysErrno + currentMaskedErrorID;
        clearErrorMask();
        advanceErrorMask(currentMaskedErrorID);
        clearError();
    }
    return errorWord;
}




