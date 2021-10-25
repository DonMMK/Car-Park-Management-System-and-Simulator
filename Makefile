CC = gcc
CFLAGS = -Wall -pedantic -pthread -lrt # Show all reasonable warnings
LDFLAGS =

all: Simulator Manager

Simulator: Simulator.c
	gcc -o Simulator Simulator.c $(CFLAGS)

Manager: Manager.c
	gcc -o Manager Manager.c $(CFLAGS)

FireAlarm: firealarm.c
	gcc -o FireAlarm firealarm.c $(CFLAGS)

clean:
	rm -f Manager Simulator FireAlarm *.o

.PHONY: all clean
