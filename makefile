#  HASP 2105 UNIVERSITY OF MINNESOTA

CC=gcc

all: main

main: main.o VN100.o serial.o timing.o
	$(CC) -o MAIN main.o VN100.o serial.o timing.o 

main.o: main.c globaldefs.h
	$(CC) -c main.c

VN100.o: VN100.c VN100.h globaldefs.h serial.h
	$(CC) -c VN100.c

serial.o: serial.c serial.h
	$(CC) -c serial.c

timing.o: timing.c timing.h
	$(CC) -c timing.c

read: read_fifo_store_data.c
	$(CC) -o read_fifo_store_data read_fifo_store_data.c

clean:
	rm -rf *.o
