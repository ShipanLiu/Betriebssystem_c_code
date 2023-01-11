CC      = gcc
CFLAGS  = -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -pthread
LDFLAGS = -pthread
RM      = rm -f

.PHONY: all clean

all: paffin

clean:
	$(RM) paffin paffin.o

paffin: paffin.o sem.o list.o
	$(CC) ${LDFLAGS} -o $@ $^

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

paffin.o: paffin.c sem.h list.h
