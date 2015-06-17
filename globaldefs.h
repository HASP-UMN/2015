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

#define BUFMAX 500

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#include <stdint.h>
#include <stdlib.h>


// DATA TYPES
typedef unsigned char   byte;           //  typedef of byte
typedef unsigned short  word;           //  typedef of word


// PHOTON DATA BUFFER (FROM FIFO) FOR IRQ HANDLER
extern const unsigned long  LENGTH = 500;
extern unsigned char        PHOTON_DATA_BUFFER[LENGTH];
extern int                  BYTES_PER_PHOTON = 10;
extern int                  PHOTONS_AQUIRED = 0;


// ISA BUS INPUT PORT
const unsigned short INPUT_PORT = 0x800; // base address
int SYNC_BYTE = 77; //arbitrarily chosen for now
int IRQ = 6;


// STRUCTURE FOR OEMSTAR DATA (BESTXYZB)
struct gps
{
    // RECEIVER CHARACTERISTICS
    int port;                       // GPS receiver serial communication port
    char responseBuffer[BUFFMAX];	// Character buffer for response data
    int readCalls;                  // Number of times read_gps has been called
    int badDataCounter;             // Number of bad read from read_gps (rests after 5)
    int posValFlag;                 // Flag signifying valid XYZ position from receiver
    unsigned long lastPosVal;       // Last position valid
    
    // GPS TIME (TELEMETRY PACKET BYTES 1-2)
    uint8_t timeStatus;             // Time status
    uint16_t weekRef;               // GPS week reference number
    long time;                      // GPS timestamp [s]
 
    // GPS ECEF X (TELEMETRY PACKET BYTES 3-9)
    double Xe;                      // X position (ECEF) [m]
    float  P_Xe;                    // X std dev [m]
    
    // GPS ECEF Y (TELEMETRY PACKET BYTES 10-18)
    double Ye;                      // Y position (ECEF) [m]
    float  P_Ye;                    // Y std dev [m]
    
    // GPS ECEF Z (TELEMETRY PACKET BYTES 19-27)
    double Ze;                      // Z position (ECEF) [m]
    float  P_Ze;                    // Z std dev [m]
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


// STRUCTURE FOR VN100 DATA (REGISTER ID: 54)
struct imu
{
    // SERIAL I/O RESPONSE
    int port;
    char dataBuffer[131];
    
    // AMBIENT PAYLOAD TEMPERATURE (TELEMETRY PACKET BYTES 73-78)
    float Temp;     // [C]
    
    // OTHER CALIBRATED SENSOR MEASUREMENTS
    float MagX;     // [gauss]
    float MagY;     // [gauss]
    float MagZ;     // [gauss]
    float AccelX;   // [m/s^2]
    float AccelY;   // [m/s^2]
    float AccelZ;   // [m/s^2]
    float GyroX;    // [rad/s]
    float GyroY;    // [rad/s]
    float GyroZ;    // [rad/s]
    // ambient temperature measurement comes here in the response packet
    float Pressure; // [kPa]
};

#endif
