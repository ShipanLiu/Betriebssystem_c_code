## TODO
# all, clean (-> als .PHONY deklarieren, da es Pseudotargets sind)
# parrots
# Zwischenprodukte (-> parrots.o)
# sem.o, queue.o und triangle.o müssen nicht erzeugt werden
# clean löscht alle Zwischenergebnisse und das Programm parrots
# CC und CFLAGS konventionskonform


.PHONY: all clean

all: parrots

CC = GCC
CFLAGS = -std=c99 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

parrots: parrots.o sem.o triangle.o queue.o
	$(CC) $(CFLAGS) -o parrots parrots.o sem.o triangle.o queue.o
	# $(CFLAGS) ist hier nicht nötig, aber auch nicht falsch
	# $(CC) $(CFLAGS) -o $@ $^

parrots.o: parrots.c sem.h queue.h triangle.h
		$(CC) $(CFLAGS) -c parrots.c

clean:
	rm -f parrots parrots.o
	
# Pseudotargets werden nicht gebaut, es gibt nachher also keine Datei mit dem Namen "all" oder "clean".
# Bei Pseudotargets wird lediglich das ausgeführt, was danach beschrieben ist.
# make all -> bei "all" steht "parrots", d.h. make all entspricht make parrots
# make clean -> rm -f parrots parrots.o wird ausgeführt, es werden also alle bisher durch das Makefile erzeugten Dateien gelöscht.
