// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// errorword.h
// This file manages errors reported by the applications and
//     creates an error word to be sent in the telemetry packet.
// Last Edited By: Luke Granlund
// Last Edited On: 14 July 2015, 15:30

#ifndef ERRORWORD_H
#define ERRORWORD_H


void init_ErrorReporting();
void init_ErrorPipe();
void reportError(ERRWD errorID);
void getErrorWord();


#endif // ERRORWORD_H
