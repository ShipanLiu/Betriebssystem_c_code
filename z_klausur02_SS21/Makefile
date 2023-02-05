GCC = gcc
CFLAG = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

all: hupsi

hupsi: hupsi.o
	${GCC} -o hupsi hupsi.o

hupsi.o: hupsi_shipan.c
	${GCC} ${CFLAG} -c hupsi_shipan.c

clean: rm -f hupsi hupsi.o

PHONY: all clean