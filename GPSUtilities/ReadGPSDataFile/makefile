#  HASP 2105 UNIVERSITY OF MINNESOTA

CC=gcc

all: main

main: ReadGPSDataFile.o
	$(CC) -o RGDF ReadGPSDataFile.o

ReadGPSDataFile.o: ReadGPSDataFile.c
	$(CC) -c ReadGPSDataFile.c

clean:
	rm -rf *.o
