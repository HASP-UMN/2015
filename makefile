CC=gcc

all: main

main: main.o VN100.o serial.o timing.o 
	$(CC) -o test main.o VN100.o serial.o timing.o 

main.o: main.c globaldefs.h
	$(CC) -c main.c

VN100.o: VN100.c VN100.h globaldefs.h
	$(CC) -c VN100.c

serial.o: serial.c serial.h
	$(CC) -c serial.c

timing.o: timing.c
	$(CC) -c timing.c

clean:
	rm -rf *.o
