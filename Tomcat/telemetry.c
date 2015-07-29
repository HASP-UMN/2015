// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// telemetry.c
// This file send telemetry data out on COM2.
// Last Edited By: Luke Granlund
// Last Edited On: 14 July 2015, 16:00

#include <stdio.h>       // IO functions
#include <stdlib.h>      // Standard C library
#include <errno.h>       // Error enumerations
#include <termios.h>     // Serial port I/O
#include <string.h>      // Character array operations
#include <math.h>        // Mathematical operations
#include <time.h>        // Time/date functions (e.g. time since UNIX epoch)
#include <fcntl.h>       // File descriptor control
#include <unistd.h>      // Symbolic constants and types
#include <sys/types.h>   // C data types

#include "globaldefs.h"
#include "errorword.h"

#define TELE_PACKET_SIZE  83
#define TELE_COM_PORT     "/dev/ttyS1"
static int TELEMETRY_PORT;

void reset_string(char *string) {
    int string_size = strlen(string);
    int i;
    for(i = 0;i < string_size; i++) {
        string[i]='\0';
    }
}

void init_telemetry() {

    // DOWNLINK UART PORT CONFIG USING TERMIOS
    struct termios tty;
    int fd;

    // OPEN SERIAL PORT FOR TX/RX, NOT CONTROLLING DEVICES
    if((fd = open(TELE_COM_PORT, O_RDWR | O_NOCTTY)) < 0) {
        fprintf(stderr,"TELEMETRY - UNABLE TO OPEN DOWNLINK PORT\n");
    }

    // GET PORT ATTRIBUTES
    if(tcgetattr(fd, &tty) < 0) {
        fprintf(stderr,"TELEMETRY - COULD NOT GET ATTRIBUTES OF SERIAL PORT\n");
    }

    // SET BAUD RATE TO B1200, 8 DATA BITS, NO PARITY, 1 STOP BIT
    if(cfsetospeed(&tty, B1200) < 0) {
        fprintf(stderr,"TELEMETRY - UNABLE TO SET BAUD RATE\n");
    }

    // SET ATTRIBUTES
    tty.c_iflag = 0;  //input modes
    tty.c_oflag = 0;  //output modes
    tty.c_lflag = 0;  //local modes
    tcsetattr(fd, TCSANOW, &tty);
    TELEMETRY_PORT = fd;

}

// Send the telemetry packet. See the final 2015 PSIP for telemetry specification.
void send_telemetry(struct imu *imuData, struct gps *gpsData, struct photons *photonData) {
    int bytes = 0;
    static char sendpacket[TELE_PACKET_SIZE];
    char * string_to_write = malloc(9*sizeof(char));
    reset_string(string_to_write);

    // HEADER (BYTES 1-2)
    sprintf(string_to_write,"M,");
    memcpy(&sendpacket[0],string_to_write,2);
    reset_string(string_to_write);

    // GPS TIME (BYTES 3-9)
    sprintf(string_to_write,"%06u,",(uint32_t)gpsData->time/1000);
    memcpy(&sendpacket[2],string_to_write,7);
    reset_string(string_to_write);

    // GPS ECEF X (BYTES 10-18)
    sprintf(string_to_write,"%08d,",(int)gpsData->Xe);
    memcpy(&sendpacket[9],string_to_write,9);
    reset_string(string_to_write);

    // GPS ECEF Y (BYTES 19-27)
    sprintf(string_to_write,"%08d,",(int)gpsData->Ye);
    memcpy(&sendpacket[18],string_to_write,9);
    reset_string(string_to_write);

    // GPS ECEF Z (BYTES 28-36)
    sprintf(string_to_write,"%08d,",(int)gpsData->Ze);
    memcpy(&sendpacket[27],string_to_write,9);
    reset_string(string_to_write);

    // CH. 1 PHOTON EVENTS (BYTES 37-45)
    sprintf(string_to_write,"%08d,",photonData->counts_ch01);
    memcpy(&sendpacket[36],string_to_write,9);
    reset_string(string_to_write);

    // CH. 2 PHOTON EVENTS (BYTES 46-54)
    sprintf(string_to_write,"%08d,",photonData->counts_ch02);
    memcpy(&sendpacket[45],string_to_write,9);
    reset_string(string_to_write);

    // CH. 3 PHOTON EVENTS (BYTES 55-63)
    sprintf(string_to_write,"%08d,",photonData->counts_ch03);
    memcpy(&sendpacket[54],string_to_write,9);
    reset_string(string_to_write);

    // CH. 4 PHOTON EVENTS (BYTES 64-72)
    sprintf(string_to_write,"%08d,",photonData->counts_ch04);
    memcpy(&sendpacket[63],string_to_write,9);
    reset_string(string_to_write);

    // AMBIENT PAYLOAD TEMPERATURE (BYTES 73-78)
    sprintf(string_to_write,"%+02.1f,",imuData->Temp);
    memcpy(&sendpacket[72],string_to_write,6);
    reset_string(string_to_write);

    // ERROR WORD (BYTES 79-81)
    sprintf(string_to_write,"%03X",ERROR_WORD);
    memcpy(&sendpacket[78],string_to_write,3);
    reset_string(string_to_write);

    // FOOTER (BYTES 82-83)
    sprintf(string_to_write,"*\n");
    memcpy(&sendpacket[81],string_to_write,2);
    reset_string(string_to_write);

    // COPY STRING (BYTE BY BYTE) TO DOWNLINK PORT
    while(bytes < TELE_PACKET_SIZE) {
        bytes += write(TELEMETRY_PORT, &sendpacket[bytes], 1);
    }

    // COPY TILDES INTO THE STRING
    for(bytes=0; bytes < strlen(string_to_write); bytes++) {
        string_to_write[bytes] = '~';
    }

    fprintf(stderr,"TELEMETRY: %s",sendpacket);  // For Debugging

}
