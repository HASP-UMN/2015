#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "globaldefs.h"

int updateEventCounter(struct photons* photonData_ptr)
{
    // Update event A counter
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t readLine;
    fp = fopen("/home/root/peakLogging/eventACounts", "r");
    if (fp == NULL)
    {
        ;// skip
    }
    else
    {
        getline(&line, &len, fp);
        photonData_ptr->countsA = atoi(line);
        //printf("%d\n", count);
    }
    // Update event B counter
    fflush(fp);
    fclose(fp);
    fp = fopen("/home/root/peakLogging/eventBCounts", "r");
    if (fp == NULL)
    {
        ;//skip
    }
    else
    {
        getline(&line, &len, fp);
        photonData_ptr->countsB = atoi(line);
        //printf("%d\n", count);
    }
    
    fflush(fp);
    fclose(fp);
    
    // Update event C counter
    fp = fopen("/home/root/peakLogging/eventCCounts", "r");
    if (fp == NULL)
    {
        ;//skip
        
    }
    else
    {
        getline(&line, &len, fp);
        photonData_ptr->countsC = atoi(line);
        //printf("%d\n", count);
    }
    
    fflush(fp);
    fclose(fp);
    
    // Update event D counter
    fp = fopen("/home/root/peakLogging/eventDCounts", "r");
    if (fp == NULL)
    {
        ;//skip
        
    }
    else
    {
        getline(&line, &len, fp);
        photonData_ptr->countsD = atoi(line);
        //printf("%d\n", count);
    }
    
    if (line)
        free(line);
    
    
    return 1;//not sure why this returns a 1 on sucess. Standard convention is to return 0
}