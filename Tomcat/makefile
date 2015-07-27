#  HASP 2105 UNIVERSITY OF MINNESOTA

CC=gcc

all: main

main: main.o VN100.o gps_novatel.o serial.o timing.o errorword.o telemetry.o
	$(CC) -o MAIN main.o VN100.o gps_novatel.o serial.o timing.o errorword.o telemetry.o
	rm -rf *.o

main.o: main.c globaldefs.h errorword.h
	$(CC) -c main.c

VN100.o: VN100.c VN100.h globaldefs.h serial.h errorword.h
	$(CC) -c VN100.c

gps_novatel.o: gps_novatel.c gps_novatel.h globaldefs.h errorword.h
	$(CC) -c gps_novatel.c

serial.o: serial.c serial.h
	$(CC) -c serial.c

timing.o: timing.c timing.h
	$(CC) -c timing.c

errorword.o: errorword.c errorword.h globaldefs.h
	$(CC) -c errorword.c

telemetry.o: telemetry.c telemetry.h globaldefs.h errorword.h
	$(CC) -c telemetry.c

read: read_fifo_store_data.c
	$(CC) -o read_fifo_store_data read_fifo_store_data.c

clean:
	$(RM) MAIN
	rm -rf *.o

cleardata:
	rm -rf *.txt
	rm -rf *.bin
	rm -rf *.raw
