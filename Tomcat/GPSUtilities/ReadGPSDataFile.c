// ***** HASP UNIVERISTY OF MINNESOTA 2015 *****
// ReadGPSDataFile.c
// This file reads from the data file of GPS data
//     and prints it to the file GPSlist, and to the
//     file GPStable.csv. Intended for testing the,
//     but with Aug. 12 edit can also parse datafile.
// Last Edited By: Josiah DeLange
// Last Edited On: 12 August 2015, 20:07

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define GPSlist "../GPSlist.txt"
#define GPStable "../GPStable.csv"
#define GPS_DATAFILE "../GPS_OEMSTAR.raw"

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

    FILE *fp, *listfile, *tablefile;
    
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


    //                     |------> List file (console): GPSlist.txt
    // GPS binary data---->|
    //                     |------> Tabulated data file: GPStable.txt

    fp = fopen(GPS_DATAFILE, "r");
    listfile  = fopen(GPSlist, "w+");
    tablefile = fopen(GPStable, "w+");

    fprintf(listfile,"\n============== GPS Data File for NovAtel OEMStar BESTXYZB Log ==============\n",packetsRead);
    fprintf(tablefile,"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                "TimeStatus","GPSWeek","GPSMS","PSolStatus","PosType","Px","Py","Pz","PxStd","PyStd","PzStd",
                "Vx","Vy","Vz","VxStd","VyStd","VzStd","NumSVs","SolSVs"); // tabulated header

    while(readNext==true){

        elementsRead = fread(buffer, 144, 1, fp);

        if(elementsRead==1){
           int idx;
            packetsRead++;

            TimeStatus = *((char *)(&buffer[13]));
            GPSWeek = *((unsigned short *)(&buffer[14]));
            GPSMS = *((unsigned int *)(&buffer[16]));
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

            //??
            //"%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\t%f\t%d\t%d\t%d\t%f\t%f\t%f\t%s\t%s\n"
            //"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
            fprintf(tablefile,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                TimeStatus,GPSWeek,GPSMS,PSolStatus,PosType,Px,Py,Pz,PxStd,PyStd,PzStd,
                Vx,Vy,Vz,VxStd,VyStd,VzStd,NumSVs,SolSVs); // tabulated lines of data

            fprintf(listfile,"========================== GPS Data File Packet %d ==========================\n",packetsRead);
            fprintf(listfile,"HEAD Time Status..........(13): %s\n",getTimeStatus(TimeStatus));
            fprintf(listfile,"HEAD GPS Week.............(14): %d\n",GPSWeek);
            fprintf(listfile,"HEAD GPS Milliseconds.....(16): %d\n",GPSMS);
            fprintf(listfile,"RSM  Error Flag Raised....(20): %d\n",GetBitMask(buffer,20,0));
            fprintf(listfile,"RSM  Temp Warning.........(20): %d\n",GetBitMask(buffer,20,1));
            fprintf(listfile,"RSM  Volt Supply Warn.....(20): %d\n",GetBitMask(buffer,20,2));
            fprintf(listfile,"RSM  Antenna Shorted......(21): %d\n",GetBitMask(buffer,20,6));
            fprintf(listfile,"RSM  CPU Overloaded.......(21): %d\n",GetBitMask(buffer,20,7));
            fprintf(listfile,"RSM  COM1 Buff Overrun....(21): %d\n",GetBitMask(buffer,20,8));
            fprintf(listfile,"RSM  AGC Warning 1........(21): %d\n",GetBitMask(buffer,20,15));
            fprintf(listfile,"RSM  AGC Warning 2........(21): %d\n",GetBitMask(buffer,20,17));
            fprintf(listfile,"RSM  Almanac/UTC Inv......(21): %d\n",GetBitMask(buffer,20,18));
            fprintf(listfile,"RSM  No Pos Sol...........(21): %d\n",GetBitMask(buffer,20,19));
            fprintf(listfile,"RSM  Clock Model Inv......(21): %d\n",GetBitMask(buffer,20,22));
            fprintf(listfile,"RSM  Soft Res Warning.....(22): %d\n",GetBitMask(buffer,20,24));
            fprintf(listfile,"GPS  Pos Sol Status.......(28): %s\n",getSolStatus(PSolStatus));
            fprintf(listfile,"GPS  Pos Type.............(32): %s\n",getSolType(PosType));
            fprintf(listfile,"GPS  Pos-X................(36): %d\n",Px);
            fprintf(listfile,"GPS  Pos-Y................(44): %d\n",Py);
            fprintf(listfile,"GPS  Pos-Z................(52): %d\n",Pz);
            fprintf(listfile,"GPS  Pos-X Std............(60): %d\n",PxStd);
            fprintf(listfile,"GPS  Pos-Y Std............(64): %d\n",PyStd);
            fprintf(listfile,"GPS  Pos-Z Std............(68): %d\n",PzStd);
            fprintf(listfile,"GPS  Vel Sol Status.......(72): %s\n",getSolStatus(VSolStatus));
            fprintf(listfile,"GPS  Vel Type.............(76): %s\n",getSolType(VelType));
            fprintf(listfile,"GPS  Vel-X................(36): %d\n",Vx);
            fprintf(listfile,"GPS  Vel-Y................(44): %d\n",Vy);
            fprintf(listfile,"GPS  Vel-Z................(52): %d\n",Vz);
            fprintf(listfile,"GPS  Vel-X Std............(60): %d\n",VxStd);
            fprintf(listfile,"GPS  Vel-Y Std............(64): %d\n",VyStd);
            fprintf(listfile,"GPS  Vel-Z Std............(68): %d\n",VzStd);
            fprintf(listfile,"GPS  Num Sats Track......(132): %d\n",VzStd);
            fprintf(listfile,"GPS  Num Sats Sol........(133): %d\n",VzStd);

        }
        else{
            readNext = false;
        }


    }

    fclose(fp);

    if(packetsRead>0){
        fprintf(listfile,"=============================================================================\n",packetsRead);
    }

    fprintf(listfile,"Packets Read: %d\n",packetsRead);
    fprintf(listfile,"=============================================================================\n\n",packetsRead);

    fclose(listfile);
    fclose(tablefile);
    return 0;
}


