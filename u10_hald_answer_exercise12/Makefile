# done by shipan liu at 04.01.2023

CFLAGS=-Wall -Werror -std=c11 -pedantic -D_XOPEN_SOURCE=700
CC=gcc


.PHONY: all clean # optional

all: test test-ref # optional


# erstellen Sie ein Makefile, welches die Targets  "test" and "test-ref" unterstützt

test: test.o halde.o
	${CC} -o test test.o halde.o

test-ref: test.o halde-ref.o
	${CC} -o test-ref test.o halde-ref.o

test.o: test.c halde.h
	${CC} ${CFLAGS} -c test.c

halde.o: halde.c halde.h
	${CC} ${CFLAGS} -c halde.c

clean:
	rm -f test test-ref test.o halde.o # optional
