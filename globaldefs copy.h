#ifndef GLOBALDEFS_H_
#define GLOBALDEFS_H_

#define NSECS_PER_SEC           1000000000                      ///< [nsec/sec] nanoseconds per second */
#define D2R                     0.017453292519943               ///< [rad] degrees to radians */
#define R2D                     57.295779513082323              ///< [deg] radians to degrees */
#define PSI_TO_KPA              6.89475729                      ///< [KPa] PSI to KPa */
#define g                       9.814                           ///< [m/sec^2] gravity */
#define g2                      19.62                           ///< [m/sec^2] 2*g */
#define PI                      3.14159265358979                ///< pi */
#define PI2                     6.28318530717958                ///< pi*2 */
#define half_pi                 1.57079632679490                ///< pi/2 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#include <stdint.h>
#include <stdlib.h>
#include "spi.h"

typedef unsigned char   byte;           ///< typedef of byte
typedef unsigned short  word;           ///< typedef of word

// buffer for storing photon data from FIFO in irq handler
extern const unsigned long LENGTH = 500;
extern unsigned char	PHOTON_DATA_BUFFER[LENGTH];
extern int BYTES_PER_PHOTON = 10;
extern int PHOTONS_AQUIRED = 0;

struct imu
{
    //unsigned long time;
    long stime;
    long mtime;
    uint16_t supply_raw;
    int gyroX_raw;
    int gyroY_raw;
    int gyroZ_raw;
    int accelX_raw;
    int accelY_raw;
    int accelZ_raw;
    int magX_raw;
    int magY_raw;
    int magZ_raw;
    int temp_raw;
    uint16_t ADC_raw;
    int *response;
    int spi_handle;
    int badDataCounter;
    unsigned long rstTime;
};

struct photons
{
    //unsigned long time;
    int ch01;
    int ch02;
    int ch03;
    int ch04;
    //int spi_handle;
};

struct gps
{
    
    uint16_t weekRef;			///< GPS week reference number
    long time;	                  	///< [sec], timestamp of GPS data
    
    //enum PosVelType posType;			///< Position Type
    uint8_t timeStatus;					///< Time status
    double Xe;					///< [m], X position, ECEF
    double Ye;					///< [m], Y position, ECEF
    double Ze;					///< [m], Z position, ECEF
    
    float  Px;  					///< [m], X std dev
    float  Py;  					///< [m], Y std dev
    float  Pz;  					///< [m], Z std dev
    
    int port;
    char responseBuffer[144];
    int readCalls;                  //number of times read_gps has been called
    int badDataCounter;             //number of bad read from read_gps (rests after 5)
    int posValFlag;
    unsigned long lastPosVal;
};

struct sensordata
{
    struct imu *imuData_ptr;                ///< pointer to imu data structure
    struct photons *photonData_ptr;		///< pointer to xray data structure
    struct gps *gpsData_ptr;                ///< pointer to gps data structure
};

#endif