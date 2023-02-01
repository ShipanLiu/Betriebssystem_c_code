GCC = gcc
GCC_FLAGS = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700 -pthread

all: patric

patric: patric.o triangle.o sem.o
	${GCC} -o patric patric.o triangle.o sem.o

patric.o: patric.c triangle.h sem.h
	${GCC} ${GCC_FLAGS} -c patric.c

triangle.o: triangle.c triangle.h
	${GCC} ${GCC_FLAGS} -c triangle.c

clean:
	rm -f patric patric.o triangle.o

.PHONY: all clean
