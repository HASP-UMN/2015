#  HASP 2105 UNIVERSITY OF MINNESOTA

CC=gcc

all: main

main: main.o VN100.o gps_novatel.o serial.o simple_gpio.o timing.o
	$(CC) -o MAIN main.o VN100.o gps_novatel.o serial.o simple_gpio.o timing.o

main.o: main.c globaldefs.h
	$(CC) -c main.c

VN100.o: VN100.c VN100.h globaldefs.h serial.h
	$(CC) -c VN100.c

gps_novatel.o: gps_novatel.c gps_novatel.h globaldefs.h simple_gpio.h
	$(CC) -c gps_novatel.c

serial.o: serial.c serial.h
	$(CC) -c serial.c

simple_gpio.o: simple_gpio.c simple_gpio.h
	$(CC) -c simple_gpio.c

timing.o: timing.c timing.h
	$(CC) -c timing.c

read: read_fifo_store_data.c
	$(CC) -o read_fifo_store_data read_fifo_store_data.c

clean:
	rm -rf *.o
