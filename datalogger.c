#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

#include "globaldefs.h"
#include "gps_novatel.h"
#include "spi.h"
#include "HASP_SPI_devices.h"
#include "datalogger.h"

static double doubleBuf[16];
static unsigned short ushortBuf[4];
char gps_filename[] = "/mnt/microSD/flightData/GPSlog00.bin";
char imu_filename[] = "/mnt/microSD/flightData/IMUlog00.txt";

int eventCopier()
{
    char * line = NULL;
    size_t len = 0;
    FILE *origin;
    FILE *destination;
    origin=fopen("/home/root/peakLogging/eventALog.bin","r");
    if(origin!=NULL)
    {
        destination = fopen("/mnt/microSD/flightData/eventALog.bin", "a");
        if(destination!=NULL)
        {
            while(getline(&line, &len, origin) != -1)
            {
                fwrite(line, sizeof(char), sizeof(line), destination);
            }
            fclose(origin);
            fclose(destination);
            if(!remove("/home/root/peakLogging/eventALog.bin"))
            {
                printf("error delete 1\n");
            }
        }
        else
        {
            fclose(origin);
        }
        
    }
    else
    {
        printf("Nope1\n");
    }
    origin=fopen("/home/root/peakLogging/eventBLog.bin", "r");
    if(origin!=NULL)
    {
        destination = fopen("/mnt/microSD/flightData/eventBLog.bin", "a");
        if(destination!=NULL)
        {
            while(getline(&line, &len, origin) != -1)
            {
                fwrite(line, sizeof(char), sizeof(line), destination);
            }
            fclose(origin);
            fclose(destination);
            if(!remove("/home/root/peakLogging/eventBLog.bin"))
            {
                printf("error delete 1\n");
            }
        }
        else
        {
            fclose(origin);
        }
        
    }
    else
    {
        printf("Nope2\n");
    }
    
    origin=fopen("/home/root/peakLogging/eventABLog.bin", "r");
    if(origin!=NULL)
    {
        destination = fopen("/mnt/microSD/flightData/eventABLog.bin", "a");
        if(destination!=NULL)
        {
            while(getline(&line, &len, origin) != -1)
            {
                fwrite(line, sizeof(char), sizeof(line), destination);
            }
            fclose(origin);
            fclose(destination);
            if(!remove("/home/root/peakLogging/eventABLog.bin"))
            {
                printf("error delete 1\n");
            }
        }
        else
        {
            fclose(origin);
        }
        
    }
    else
    {
        printf("Nope3\n");
    }
    
    
    if(line)
        free(line);
    
    return 1;
}

int GPSlogger(struct gps* gpsData_ptr, uint8_t log_number)
{
    
    int nameLength = strlen(gps_filename);
    gps_filename[nameLength-6] = log_number/10 + '0';
    gps_filename[nameLength-5] = log_number%10 + '0';
    
    //Opening the files as "a" means append if exists
    FILE *gpsFile = fopen(gps_filename,"a");
    if(gpsFile == NULL)
    {
        fprintf(stderr,"gpsFile=%d\n",gpsFile);
        fprintf(stderr,"error opening gpsFile. exiting GPSlogger now\n");
        return -1;
    }
    
    //Log GPS buffer into binary file
    fwrite(gpsData_ptr->responseBuffer,1,144,gpsFile);
    fflush(gpsFile);
    fclose(gpsFile);
    
    return 0;
}

int IMUlogger(struct imu *imuData_ptr, uint8_t log_number)
{
    
    size_t nameLength = strlen(imu_filename);
    imu_filename[nameLength-6] = log_number/10 + '0';
    imu_filename[nameLength-5] = log_number%10 + '0';
    FILE *imuFile = fopen(imu_filename,"a");
    if(!imuFile){
        fprintf(stderr,"imuFile=%d\n", (int) imuFile);
        fprintf(stderr,"error opening IMU file. exiting now\n");
        return -1;
    }
    
    //Output data from the IMU is stored in 32-bit integers, so the
    //  file containing output will contain the RAW format data;
    //  formatted into ASCII, comma-delimited, one reading per line.
    //  We did this to enable easy parsing by MATLAB, etc once the
    //  data files are ready to be read and parsed. One set per line
    //fprintf(imuFile,"%lu,",imuData_ptr->time);
    fprintf(imuFile,"%ld,",imuData_ptr->stime);
    fprintf(imuFile,"%ld,",imuData_ptr->mtime);
    fprintf(imuFile,"%d,",imuData_ptr->supply_raw);
    fprintf(imuFile,"%d,",imuData_ptr->gyroX_raw);
    fprintf(imuFile,"%d,",imuData_ptr->gyroY_raw);
    fprintf(imuFile,"%d,",imuData_ptr->gyroZ_raw);
    fprintf(imuFile,"%d,",imuData_ptr->accelX_raw);
    fprintf(imuFile,"%d,",imuData_ptr->accelY_raw);
    fprintf(imuFile,"%d,",imuData_ptr->accelZ_raw);
    fprintf(imuFile,"%d,",imuData_ptr->magX_raw);
    fprintf(imuFile,"%d,",imuData_ptr->magY_raw);
    fprintf(imuFile,"%d,",imuData_ptr->magZ_raw);
    fprintf(imuFile,"%d,",imuData_ptr->temp_raw);
    fprintf(imuFile,"%d\n",imuData_ptr->ADC_raw);
    
    //fprintf(stderr,"IMU set written to %s\n",imu_filename);
    
    //Close the files after the logging. The next datalogger() call will append
    //  the new set of data to the existing files if log_number doesn't change.
    
    fflush(imuFile);
    fclose(imuFile);
    
    return 0;
}