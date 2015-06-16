#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

long get_timestamp_ms()
{
    
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    
    // convert tv_sec & tv_usec to millisecond
    long time_ms = (tv.tv_sec)*1000 + (tv.tv_usec)/1000 ;
    
    return time_ms;
}

long get_timestamp_us()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    
    // convert tv_sec & tv_usec to microsecond
    long time_us = (tv.tv_sec)*1000000 + (tv.tv_usec);
    
    return time_us;
}