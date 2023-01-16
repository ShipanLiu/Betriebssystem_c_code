CC      = gcc
CFLAGS  = -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -pthread
LDFLAGS = -pthread
RM      = rm -f

.PHONY: all clean

all: palim

clean:
	$(RM) palim *.o

palim: palim.o sem.o
	$(CC) ${LDFLAGS} -o $@ $^

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

palim.o: palim.c sem.h
