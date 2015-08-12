// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// ReadGPSDataFile.c
// This file reads from the data file of GPS data
//     and prints it to the screen. Intended for
//     testing the GPS.
// Last Edited By: Luke Granlund
// Last Edited On: 13 July 2015, 16:00

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define GPS_DATAFILE "../GPS_OEMSTAR.raw"
#define GPS_OUTFILE "../GPS_OUT.txt"

const char *getTimeStatus(unsigned char TimeStatus){
    switch(TimeStatus){
        case 20:
            return "UNKNOWN - Time validity is unknown";
            break;
        case 60:
            return "APPROXIMATE - Time is set approximately";
            break;
        case 80:
            return "COARSEADJUSTING -  Time is approaching course precision";
            break;
        case 100:
            return "COARSE -  This time is valid to coarse precision";
            break;
        case 120:
            return "COARSESTEERING - Time is coarse set, and is being steered";
            break;
        case 130:
            return "FREEWHEELING - Position is lost, and the range bias cannot be calculated";
            break;
        case 140:
            return "FINEADJUSTING - Time is adjusting to fine precision";
            break;
        case 160:
            return "FINE - Time has fine precision";
            break;
        case 170:
            return "FINEBACKUPSTEERING - Time is fine set and is being steered by the backup system";
            break;
        case 180:
            return "FINESTEERING - Time is fin-set and is being steered";
            break;
        case 200:
            return "SATTIME - Time from satellite";
            break;
        default:
            return "GPSUtil ERROR";
            break;
    }
    return "";
}


const char *getSolStatus(int PSolStatus){
    switch(PSolStatus){
        case 0:
            return "Solution computed";
            break;
        case 1:
            return "Insufficient observations";
            break;
        case 2:
            return "No convergence";
            break;
        case 3:
            return "Singularity at parameters matrix";
            break;
        case 4:
            return "Covariance trace exceeds maximum (trace > 1000 m)";
            break;
        case 5:
            return "Test distance exceeded (maximum of 3 rejections if distance > 10 km)";
            break;
        case 6:
            return "Not yet converged from cold start";
            break;
        case 7:
            return "Height or velocity limits exceeded (in accordance with export licensing restrictions)";
            break;
        case 8:
            return "Variance exceeds limits";
            break;
        case 9:
            return "Residuals are too large";
            break;
        case 10:
            return "Delta position is too large";
            break;
        case 11:
            return "Negative variance";
            break;
        case 13:
            return "Large residuals make position unreliable";
            break;
        case 18:
            return "Pending";
            break;
        case 19:
            return "Invalid Fix";
            break;
        case 21:
            return "Antenna warnings";
            break;
        default:
            return "GPSUtil ERROR";
            break;
    }
    return "";
}


const char *getSolType(int PosType){
    switch(PosType){
        case 0:
            return "No Solution";
            break;
        case 1:
            return "Position has been fixed by the FIX POSITION command";
            break;
        case 2:
            return "Position has been fixed by the FIX HEIGHT/AUTO command";
            break;
        case 8:
            return "Velocity computed using instantaneous Doppler";
            break;
        case 16:
            return "Single point position";
            break;
        case 17:
            return "Pseudorange differential solution";
            break;
        case 18:
            return "Solution calculated using corrections from an SBAS";
            break;
        case 19:
            return "Propagated by a Kalman filter without new observations";
            break;
        default:
            return "GPSUtil ERROR";
            break;
    }
    return "";
}


// Gets a specific bit from a set of bytes.
// Used for reading the Receiver Status Mask.
bool GetBitMask(unsigned char *bits, int startingByte, int bitMask)
{
    while(bitMask > 7)
    {
        bitMask -= 8;
        startingByte++;
    }
    return (bits[startingByte] >> bitMask) & 1;
}



int main(){

    bool readNext = true;
    int packetsRead = 0;
    int elementsRead = 0;

    FILE *fp, *out_handle;
    
    //char c[] = "this is tutorialspoint";
    unsigned char buffer[144];


    unsigned char TimeStatus;  // Binary Offset 13
    unsigned short GPSWeek;    // Binary Offset 14
    unsigned int GPSMS;        // Binary Offset 16
    int PSolStatus = 0;        // Binary Offset 28
    int PosType = 0;           // Binary Offset 32
    double Px = 0;             // Binary Offset 36
    double Py = 0;             // Binary Offset 44
    double Pz = 0;             // Binary Offset 52
    float PxStd = 0;           // Binary Offset 60
    float PyStd = 0;           // Binary Offset 64
    float PzStd = 0;           // Binary Offset 68
    int VSolStatus = 0;        // Binary Offset 72
    int VelType = 0;           // Binary Offset 76
    double Vx = 0;             // Binary Offset 80
    double Vy = 0;             // Binary Offset 88
    double Vz = 0;             // Binary Offset 96
    float VxStd = 0;           // Binary Offset 104
    float VyStd = 0;           // Binary Offset 108
    float VzStd = 0;           // Binary Offset 112
    unsigned char NumSVs;      // Binary Offset 132
    unsigned char SolSVs;      // Binary Offset 133


    fp = fopen(GPS_DATAFILE, "r");
    out_handle = fopen (GPS_OUTFILE, "w+");

    fprintf(out_handle,"\n============== GPS Data File for NovAtel OEMStar BESTXYZB Log ==============\n",packetsRead);

    while(readNext==true){

        elementsRead = fread(buffer, 144, 1, fp);

        if(elementsRead==1){
           int idx;
            packetsRead++;

            TimeStatus = *((char *)(&buffer[13]));
            GPSWeek = *((unsigned short *)(&buffer[14]));
            GPSMS = *((unsigned int *)(&buffer[16]));
            PSolStatus = *((int *)(&buffer[28]));
            PSolStatus = *((int *)(&buffer[28]));
            PosType = *((int *)(&buffer[32]));
            Px = *((double *)(&buffer[36]));
            Py = *((double *)(&buffer[44]));
            Pz = *((double *)(&buffer[52]));
            PxStd = *((float *)(&buffer[60]));
            PyStd = *((float *)(&buffer[64]));
            PzStd = *((float *)(&buffer[68]));
            Vx = *((double *)(&buffer[36]));
            Vy = *((double *)(&buffer[44]));
            Vz = *((double *)(&buffer[52]));
            VxStd = *((float *)(&buffer[60]));
            VyStd = *((float *)(&buffer[64]));
            VzStd = *((float *)(&buffer[68]));
            NumSVs = *((unsigned char *)(&buffer[132]));
            SolSVs = *((unsigned char *)(&buffer[133]));

            fprintf(out_handle,"========================== GPS Data File Packet %d ==========================\n",packetsRead);
            fprintf(out_handle,"HEAD Time Status..........(13): %s\n",getTimeStatus(TimeStatus));
            fprintf(out_handle,"HEAD GPS Week.............(14): %d\n",GPSWeek);
            fprintf(out_handle,"HEAD GPS Milliseconds.....(16): %d\n",GPSMS);
            fprintf(out_handle,"RSM  Error Flag Raised....(20): %d\n",GetBitMask(buffer,20,0));
            fprintf(out_handle,"RSM  Temp Warning.........(20): %d\n",GetBitMask(buffer,20,1));
            fprintf(out_handle,"RSM  Volt Supply Warn.....(20): %d\n",GetBitMask(buffer,20,2));
            fprintf(out_handle,"RSM  Antenna Shorted......(21): %d\n",GetBitMask(buffer,20,6));
            fprintf(out_handle,"RSM  CPU Overloaded.......(21): %d\n",GetBitMask(buffer,20,7));
            fprintf(out_handle,"RSM  COM1 Buff Overrun....(21): %d\n",GetBitMask(buffer,20,8));
            fprintf(out_handle,"RSM  AGC Warning 1........(21): %d\n",GetBitMask(buffer,20,15));
            fprintf(out_handle,"RSM  AGC Warning 2........(21): %d\n",GetBitMask(buffer,20,17));
            fprintf(out_handle,"RSM  Almanac/UTC Inv......(21): %d\n",GetBitMask(buffer,20,18));
            fprintf(out_handle,"RSM  No Pos Sol...........(21): %d\n",GetBitMask(buffer,20,19));
            fprintf(out_handle,"RSM  Clock Model Inv......(21): %d\n",GetBitMask(buffer,20,22));
            fprintf(out_handle,"RSM  Soft Res Warning.....(22): %d\n",GetBitMask(buffer,20,24));
            fprintf(out_handle,"GPS  Pos Sol Status.......(28): %s\n",getSolStatus(PSolStatus));
            fprintf(out_handle,"GPS  Pos Type.............(32): %s\n",getSolType(PosType));
            fprintf(out_handle,"GPS  Pos-X................(36): %d\n",Px);
            fprintf(out_handle,"GPS  Pos-Y................(44): %d\n",Py);
            fprintf(out_handle,"GPS  Pos-Z................(52): %d\n",Pz);
            fprintf(out_handle,"GPS  Pos-X Std............(60): %d\n",PxStd);
            fprintf(out_handle,"GPS  Pos-Y Std............(64): %d\n",PyStd);
            fprintf(out_handle,"GPS  Pos-Z Std............(68): %d\n",PzStd);
            fprintf(out_handle,"GPS  Vel Sol Status.......(72): %s\n",getSolStatus(VSolStatus));
            fprintf(out_handle,"GPS  Vel Type.............(76): %s\n",getSolType(VelType));
            fprintf(out_handle,"GPS  Vel-X................(36): %d\n",Vx);
            fprintf(out_handle,"GPS  Vel-Y................(44): %d\n",Vy);
            fprintf(out_handle,"GPS  Vel-Z................(52): %d\n",Vz);
            fprintf(out_handle,"GPS  Vel-X Std............(60): %d\n",VxStd);
            fprintf(out_handle,"GPS  Vel-Y Std............(64): %d\n",VyStd);
            fprintf(out_handle,"GPS  Vel-Z Std............(68): %d\n",VzStd);
            fprintf(out_handle,"GPS  Num Sats Track......(132): %d\n",VzStd);
            fprintf(out_handle,"GPS  Num Sats Sol........(133): %d\n",VzStd);

        }
        else{
            readNext = false;
        }


    }

    fclose(fp);

    if(packetsRead>0){
        fprintf(out_handle,"=============================================================================\n",packetsRead);
    }

    fprintf(out_handle,"Packets Read: %d\n",packetsRead);
    fprintf(out_handle,"=============================================================================\n\n",packetsRead);

    return 0;
}


