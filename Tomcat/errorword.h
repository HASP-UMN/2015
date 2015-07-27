// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// errorword.h
// This file manages errors reported by the applications and
//     creates an error word to be sent in the telemetry packet.
// Last Edited By: Luke Granlund
// Last Edited On: 14 July 2015, 15:30

#ifndef ERRORWORD_H
#define ERRORWORD_H

/*
    Enumeration of errors that can be recorded in HASP.
    The errors are listed in decreasing priority. The
    specification for the errors is on the 2015 HASP
    Google Drive in the Telemetry folder.
*/
typedef enum ERRWD {
    ERR_NO_ERROR,
    ERR_GPS_PORTOPEN,
    ERR_GPS_PORTGATT,
    ERR_GPS_PORTOBR,
    ERR_GPS_PORTIBR,
    ERR_GPS_PORTSATT,
    ERR_IMU_PORTOPEN,
    ERR_IMU_WRITEFO,
    ERR_GPS_READ,
    ERR_GPS_READSYNC,
    ERR_GPS_READBYTES,
    ERR_GPS_READPACK,
    ERR_GPS_READCHECK,
    ERR_GPS_WRITEFO,
    ERR_GPS_WRITE,
    ERR_GPS_RSMEFR,
    ERR_GPS_RSMTEMP,
    ERR_GPS_RSMVOLT,
    ERR_GPS_RSMANT,
    ERR_GPS_RSMCPU,
    ERR_GPS_RSMCOM,
    ERR_GPS_RSMAGC,
    ERR_GPS_RSMALM,
    ERR_GPS_RSMPOS,
    ERR_GPS_RSMCLO,
    ERR_GPS_RSMSRW,
    ERR_IMU_READ,
    ERR_IMU_READBYTES,
    ERR_IMU_READCHECK,
    ERR_IMU_WRITE
}
ERRWD;


void reportError(ERRWD errorID);
void getErrorWord();


#endif // ERRORWORD_H
