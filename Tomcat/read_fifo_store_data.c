#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
//#include <resource.h>

#include "globaldefs.h"

int read_fifo_store_data(int fifo_fd, int storage_fd, unsigned char* buf, size_t length);

int main(int argc, char** argv)
{
	int fifo_fd, storage_fd;
	fifo_fd = open("/dev/FIFO_DEV", O_RDONLY);
	fprintf(stderr, "/dev/FIFO_DEV opened: fd = %d\n", fifo_fd);
	storage_fd = open("photon_data.txt", O_RDWR | O_CREAT);
	if (storage_fd == -1){
		perror(NULL);
	}
	fprintf(stderr, "photon_data.txt opened: fd = %d\n", storage_fd);

	unsigned char photon_data[BUFMAX*4];
	if ( setpriority(0, 0, 0) < 0 )
	{
		fprintf(stderr, "Failed to set child process' priority to 0\n");
	}
	fprintf(stderr, "Child's priority is: %d \n", getpriority(0, 0) );

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
	int count = 0;
	fprintf(stderr, "Child process %i has entered read_fifo_store_data\n", getpid() );

	int bytes_read;
	while(count < 10){
		if ( (bytes_read = read(fifo_fd, buf, length)) != length) //reading from fifo_device
        {
            fprintf(stderr, "Error: couldn't read from fifo_dev correctly; bytes_read = %d\n", bytes_read);
        }
//		fprintf(stderr, "returned from read \n");
        // now store the data in buf to a file
		buf[BUFMAX] = '\n';
        write(storage_fd, buf, bytes_read+1);
		fprintf(stderr, "Child read from fifo. \n");
		count++;
    }
	close(storage_fd);
	close(fifo_fd);
    return 0;
}
