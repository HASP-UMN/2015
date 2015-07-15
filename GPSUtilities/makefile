#  HASP 2105 UNIVERSITY OF MINNESOTA

CC=gcc

all: RGDF

RGDF: ReadGPSDataFile.o
	$(CC) -o RGDF ReadGPSDataFile.o
	rm -rf *.o

ReadGPSDataFile.o: ReadGPSDataFile.c
	$(CC) -c ReadGPSDataFile.c

clean:
	$(RM) RGDF
	rm -rf *.o
