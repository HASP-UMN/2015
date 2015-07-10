

#include <stdlib.h>      // Standard C Library
#include <stdio.h>       // Standard I/O Library
#include <stdbool.h>     // Standard C Library for Boolean Capability
#include <errno.h>       // System Error Enumerations
#include <math.h>        // Mathematical operations

#include "errorword.h"


#define ERRNO_BITSHIFT 8

#define ERR_MASK_MAX 10


unsigned int currentMaskedErrorID = 0;
unsigned int currentUnmaskedErrorID = 0;
unsigned int currentMaskedSysErrno = 0;
unsigned int currentUnmaskedSysErrno = 0;
unsigned int maskedErrors[ERR_MASK_MAX] = {0,0,0,0,0,0,0,0,0,0};



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


void reportError(ERRWD errorID){
    if(checkErrorMask(errorID)==false){
        if(errorID < currentUnmaskedErrorID || currentUnmaskedErrorID == 0){
            currentUnmaskedErrorID = errorID;
            currentUnmaskedSysErrno = errno;
            errno = 0;
        }
    }
    else{

        //addReceivedMaskedError(errorID,errno);

        if(errorID < currentMaskedErrorID || currentMaskedErrorID == 0){
            currentMaskedErrorID = errorID;
            currentMaskedSysErrno = errno;
            errno = 0;
        }
    }
    // For Debugging
    fprintf(stderr,"\n*********************************\n");
    fprintf(stderr,"    HASP ERROR: %d\n",errorID);
    fprintf(stderr,"         ERRNO: %d\n",errno);
    fprintf(stderr,"*********************************\n\n");
}


int getErrorWord(){
    int errorWord = 0;
    int sysErrno = 0;
    if(currentUnmaskedErrorID!=0){
        sysErrno = currentUnmaskedSysErrno << ERRNO_BITSHIFT;
        errorWord = sysErrno + currentUnmaskedErrorID;
        advanceErrorMask(currentUnmaskedErrorID);
        clearError();
    }
    else if(currentMaskedErrorID!=0){

        //getMaskedError();

        sysErrno = currentMaskedSysErrno << ERRNO_BITSHIFT;
        errorWord = sysErrno + currentMaskedErrorID;

        int idx;
        for(idx=0;idx<10;idx++){
            maskedErrors[idx] = 0;
        }

        advanceErrorMask(currentMaskedErrorID);
        clearError();

    }
    return errorWord;
}




