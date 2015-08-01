// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// errorpipe.h
// This file provides a pipe to send errors from the
//     fifo process to the main process.

#ifndef ERRORPIPE_H
#define ERRORPIPE_H


void reportError(ERRWD errorID);
void init_ErrorPipe();


#endif // ERRORPIPE_H
