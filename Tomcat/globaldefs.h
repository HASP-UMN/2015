#ifndef GLOBALDEFS_H_
#define GLOBALDEFS_H_

#define NSECS_PER_SEC           1000000000                      ///< [nsec/sec] nanoseconds per second */
#define D2R                     0.017453292519943               ///< [rad] degrees to radians */
#define R2D                     57.295779513082323              ///< [deg] radians to degrees */
#define g                       9.814                           ///< [m/sec^2] gravity */
#define g2                      19.62                           ///< [m/sec^2] 2*g */
#define PI                      3.14159265358979                ///< pi */
#define PI2                     6.28318530717958                ///< pi*2 */
#define half_pi                 1.57079632679490                ///< pi/2 */

#define BUFMAX 504

#define ERROR_FIFO "ErrorPipe.fifo"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


// STRUCTURE FOR OEMSTAR DATA (BESTXYZB)
struct gps
{
    // GPS I/O
    int gps_fd;                     // GPS receiver serial communication file descriptor
    FILE* GPSDataFile;              // GPS file to write data to

    // RECEIVER CHARACTERISTICS
    bool lastPosVal;                // Last position valid

    // GPS TIME (TELEMETRY PACKET BYTES 1-2)
    uint8_t timeStatus;             // Time status
    uint16_t weekRef;               // GPS week reference number
    long time;                      // GPS timestamp [s]

    // GPS ECEF X (TELEMETRY PACKET BYTES 3-9)
    double Xe;                      // X position (ECEF) [m]

    // GPS ECEF Y (TELEMETRY PACKET BYTES 10-18)
    double Ye;                      // Y position (ECEF) [m]

    // GPS ECEF Z (TELEMETRY PACKET BYTES 19-27)
    double Ze;                      // Z position (ECEF) [m]
};



// STRUCTURE FOR PHOTON DATA (FROM CMOS AYNCHRONOUS FIFO, VIA THE ISA BUS)
struct photons
{
    // CHANNEL 1 PHOTON EVENTS (TELEMETRY PACKET BYTES 37-45)
    int counts_ch01;    // number of gamma photons stopped by detector 1

    // CHANNEL 2 PHOTON EVENTS (TELEMETRY PACKET BYTES 46-54)
    int counts_ch02;    // number of gamma photons stopped by detector 2

    // CHANNEL 3 PHOTON EVENTS (TELEMETRY PACKET BYTES 55-63)
    int counts_ch03;    // number of gamma photons stopped by detector 3

    // CHANNEL 4 PHOTON EVENTS (TELEMETRY PACKET BYTES 64-72)
    int counts_ch04;    // number of gamma photons stopped by detector 4
};

//STRUCTURE FOR VN100 DATA (REGISTER ID: 54)
struct imu
{

    // GPS I/O
    int imu_fd;                     // GPS receiver serial communication file descriptor
    FILE* VN100File;                // GPS file to write data to
    char dataBuffer[130];

    // AMBIENT PAYLOAD TEMPERATURE (TELEMETRY PACKET BYTES 73-78)
    float Temp;     // [C]

};


// ERROR WORD (TELEMETRY PACKET BYTES 79-81)
unsigned short ERROR_WORD;




/*
    Enumeration of errors that can be recorded in HASP.
    The errors are listed in decreasing priority. The
    specification for the errors is on the 2015 HASP
    Google Drive in the Telemetry folder.
*/
typedef enum ERRWD {
    ERR_NO_ERROR,
    ERR_TEST,
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
    ERR_IMU_WRITE,
    ERR_ER_RMFIFO,
    ERR_ER_MKFIFO,
    ERR_ER_OPIPE,
    ERR_ER_RDPIPE,
    ERR_ER_PIPEBYTES,
    ERR_GPS_TIME
}
ERRWD;


#endif
