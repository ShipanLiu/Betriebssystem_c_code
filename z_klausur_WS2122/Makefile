## finished by Shipan Liu
## parrots
## zwischenproduckt
## sem.o queue.o und triangle.o muessen nicht erzeugt werden
## clean loescht alle Zwischenergebnisse und das Program parrots
## CC und CFLAGS

CC = gcc
CFLAGS  = -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -pthread

all: parrots

parrots: parrots.o sem.o triangle.o queue.o
		${CC} -o parrots parrots.o sem.o triangle.o queue.o

parrots.o: parrots.c sem.h tringle.h queue.h
		${CC} ${CFLAGS} -c parrots.c

clean: rm -f parrots parrots.o

.PHONY: all clean
