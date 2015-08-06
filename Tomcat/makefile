#  HASP 2105 UNIVERSITY OF MINNESOTA
flags = -Wall -g
CC=gcc

all: read main

assembly: main -S

main: main.o VN100.o gps_novatel.o serial.o timing.o errorword.o telemetry.o
	$(CC) $(flags) -o main main.o VN100.o gps_novatel.o serial.o timing.o errorword.o telemetry.o

read: read_fifo_store_data.o errorpipe.o telepipe.o
	$(CC) $(flags) -o read_fifo_store_data read_fifo_store_data.o errorpipe.o telepipe.o

main.o: main.c globaldefs.h errorword.h
	$(CC) $(flags) -c main.c

VN100.o: VN100.c VN100.h globaldefs.h serial.h errorword.h
	$(CC) $(flags) -c VN100.c

gps_novatel.o: gps_novatel.c gps_novatel.h globaldefs.h errorword.h
	$(CC) $(flags) -c gps_novatel.c

serial.o: serial.c serial.h
	$(CC) $(flags) -c serial.c

timing.o: timing.c timing.h
	$(CC) $(flags) -c timing.c

errorword.o: errorword.c errorword.h globaldefs.h
	$(CC) $(flags) -c errorword.c

errorpipe.o: errorpipe.c errorpipe.h globaldefs.h
	$(CC) $(flags) -c errorpipe.c

telemetry.o: telemetry.c telemetry.h globaldefs.h errorword.h
	$(CC) $(flags) -c telemetry.c

telepipe.o: telepipe.c telepipe.h globaldefs.h
	$(CC) $(flags) -c telepipe.c

read_fifo_store_data.o: read_fifo_store_data.c globaldefs.h errorpipe.h telepipe.h
	$(CC) $(flags) -c read_fifo_store_data.c

clean:
	rm -rf *.o

cleardata:
	rm -rf *.txt
	rm -rf *.bin
	rm -rf *.raw
