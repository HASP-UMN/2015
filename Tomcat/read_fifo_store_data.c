#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
//#include <resource.h>

#include "globaldefs.h"
#include "errorpipe.h"
#include "telepipe.h"

//int read_fifo_store_data(int fifo_fd, int storage_fd, unsigned char* buf, size_t length);
//void checksum(unsigned char*, unsigned char *);
//void determineError(char* pass_fail);

#define PACKETS 42

void read_channels();
int read_fifo_store_data(int fifo_fd, int storage_fd, unsigned char* buf, size_t length);
void calculate_checksum();
void determineError();

unsigned char photon_data[BUFMAX];
unsigned int pass_fail[PACKETS];


int main(int argc, char** argv)
{
    init_ErrorPipe();
    init_telemetryPipe();
    reportError(ERR_TEST);

    //recordChannel(1);
    //recordChannel(1);
    //recordChannel(1);


	int fifo_fd, storage_fd;
	fifo_fd = open("/dev/FIFO_DEV", O_RDONLY);
	fprintf(stderr, "/dev/FIFO_DEV opened: fd = %d\n", fifo_fd);
	storage_fd = open("data/photon_data.txt", O_RDWR | O_CREAT | O_APPEND);
	if (storage_fd == -1){
		perror(NULL);
	}
	fprintf(stderr, "photon_data.txt opened: fd = %d\n", storage_fd);

	if ( setpriority(0, 0, 0) < 0 )
	{
		fprintf(stderr, "Failed to set child process' priority to 0\n");
	}
//	fprintf(stderr, "Child's priority is: %d \n", getpriority(0, 0) );

//	unsigned char photon_data[BUFMAX];
	bzero(pass_fail, PACKETS);

	int ret = read_fifo_store_data(fifo_fd, storage_fd, photon_data, BUFMAX);
	if (ret != 0)
	{
		fprintf(stderr, "Problem with read_fifo_store_data.c: Didn't return 0\n");
	}
	fprintf(stderr,"child returned from read_fifo_store_data.c: ret = %d\n", ret);

	return ret;
}


int read_fifo_store_data(int fifo_fd, int storage_fd, unsigned char* buf, size_t length)
{
	fprintf(stderr, "Child process %i has entered read_fifo_store_data\n", getpid() );
	int bytes_read;

	while(1){
		if ( (bytes_read = read(fifo_fd, buf, length)) != length) //reading from fifo_device
        	{
            		fprintf(stderr, "Error: couldn't read from fifo_dev correctly; bytes_read = %d\n", bytes_read);
        	}

        // now store the data in buf to a file
		calculate_checksum();
		read_channels();
		determineError(pass_fail);
		write(storage_fd, buf, bytes_read);
		fprintf(stderr, "Child read from fifo. \n");
    }
	close(storage_fd);
	close(fifo_fd);
    	return 0;
}



void calculate_checksum(){

	int i;
	int counter = 0;
	int packet = 0;
	int sum = 0;
	int sum_recvd = 0;

	for(i = 0; i < BUFMAX; i++){
		counter++;
		if (counter == 11){
			sum_recvd = photon_data[i] << 8; //assuming here that checksum bytes are stored little endian . Depends on how they're sent by 2560
			sum_recvd += photon_data[i+1];
			if (sum_recvd != sum){
				pass_fail[packet] = 1; //flag a failure the rest are left as zero so pass_fail must be preinitialized to 0's
			}
		}
		if (counter == 12){
			counter = 0;
			sum = 0;
			packet++;
		}
		else{
			sum += (int) photon_data[i];
		}
	}
}


void determineError(){
    int totalFailed = 0;
    int i;
	size_t length;
	length = PACKETS;
    for(i=0;i<length;i++){
        totalFailed = totalFailed + pass_fail[i];
    }
    if(totalFailed == 42){
        reportError(ERR_DET_CHKALL);
    }
    else if(totalFailed > 20){
        reportError(ERR_DET_CHKHALF);
    }
    else if(totalFailed > 0){
        reportError(ERR_DET_CHK);
    }
    bzero(pass_fail, 42);
}

// include telepipe
void read_channels(){
	int i;
	for(i = 0; i < BUFMAX; i=i+12){
        recordChannel(photon_data[i]);
	}
	//recordChannel(1);
}

