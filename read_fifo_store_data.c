#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int read_fifo_store_data(int fifo_fd, int storage_fd, unsigned char* buf, size_t length)
{
    fprintf(stderr, "Child process %i has entered read_fifo_store_data\n", getpid() );
    
    while(1){
        
        int bytes_read;
        if ( (bytes_read = read(fifo_fd, buf, length)) < 0) //reading from fifo_device
        {
            fprintf(stderr, "Error: couldn't read from fifo_dev\n");
        }
        
        // now store the data in buf to a file
        write(storage_fd, buf, bytes_read);
    
    }
}