#ifndef ERRORWORD_H
#define ERRORWORD_H


typedef enum ERRWD {
    ERR_NO_ERROR,
    ERR_IMU_PORTOPEN,
    ERR_IMU_READ,
    ERR_IMU_READBYTES,
    ERR_IMU_READCHECK,
    ERR_IMU_WRITEFO,
    ERR_IMU_WRITE,
    ERR_GPS_PORTOPEN,
    ERR_GPS_PORTGATT,
    ERR_GPS_PORTOBR,
    ERR_GPS_PORTIBR,
    ERR_GPS_PORTSATT,
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
    ERR_GPS_RSMSRW
}
ERRWD;



extern unsigned int currentMaskedErrorID ;
extern unsigned int currentUnmaskedErrorID ;
extern unsigned int currentMaskedSysErrno ;
extern unsigned int currentUnmaskedSysErrno ;
extern unsigned int maskedErrors[10];



void advanceErrorMask(unsigned int errorID);
bool checkInErrorMask(unsigned int errorID);
void clearError();
void reportError(ERRWD errorID);
int getErrorWord();



#endif // ERRORWORD_H
