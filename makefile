CC=gcc

all: lokanta

lokanta: lokanta.c
	$(CC) -o lokanta lokanta.c -pthread

