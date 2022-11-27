GCC = gcc
GCC_FLAGS = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

all: clash

clash: clash.o plist.o
	${GCC} -o clash clash.o plist.o

clash.o: clash.c plist.h
	${GCC} ${GCC_FLAGS} -c clash.c

plist.o: plist.c plist.h
	${GCC} ${GCC_FLAGS} -c plist.c

clean:
	rm -f clash clash.o plist.o

.PHONY: all clean
